#include <jansson.h>
#include <string.h>
#include <stdarg.h>
#include "util.h"


#define INPUT "{ \"testkey\": [\"testvalue1\", \"testvalue2\"] }"
#define MULTILINE_INPUT						\
"{\n"								\
"	\"root key 1\": \"root key 1 value 1\",\n"		\
"	\"root key 2\": [\n"					\
"		\"root key 2 array value 1\",\n"		\
"		\"root key 2 array value 2\"\n"			\
"	],\n"							\
"	\"root key 3\": [ true, false ],\n"			\
"	\"root key 4\": null,\n"				\
"	\"root key 5\": {\n"					\
"		\"root key 5 object key 1\": 23,\n"		\
"		\"root key 5 object key 2\": 3.1415926536\n"	\
"	},\n"							\
"	\"root key emoji\": \"\\uD83D\\uDE02\"\n"		\
"}\n"

#define OUTPUT					\
"object: 1, 1, 1, 1\n"				\
"object key \"testkey\" value: 1, 14, 14, 1\n"	\
"array: 1, 14, 14, 1\n"				\
"array item 0: 1, 15, 15, 12\n"			\
"string: \"testvalue1\": 1, 15, 15, 12\n"	\
"array item 1: 1, 29, 29, 12\n"			\
"string: \"testvalue2\": 1, 29, 29, 12\n"

#define MULTILINE_OUTPUT						\
"object: 1, 1, 1, 1\n"							\
"object key \"root key 1\" value: 2, 16, 18, 20\n"			\
"string: \"root key 1 value 1\": 2, 16, 18, 20\n"			\
"object key \"root key 2\" value: 3, 16, 55, 1\n"			\
"array: 3, 16, 55, 1\n"							\
"array item 0: 4, 3, 59, 26\n"						\
"string: \"root key 2 array value 1\": 4, 3, 59, 26\n"			\
"array item 1: 5, 3, 89, 26\n"						\
"string: \"root key 2 array value 2\": 5, 3, 89, 26\n"			\
"object key \"root key 3\" value: 7, 16, 135, 1\n"			\
"array: 7, 16, 135, 1\n"						\
"array item 0: 7, 18, 137, 4\n"						\
"true: 7, 18, 137, 4\n"							\
"array item 1: 7, 24, 143, 5\n"						\
"false: 7, 24, 143, 5\n"						\
"object key \"root key 4\" value: 8, 16, 167, 4\n"			\
"null: 8, 16, 167, 4\n"							\
"object key \"root key 5\" value: 9, 16, 188, 1\n"			\
"object: 9, 16, 188, 1\n"						\
"object key \"root key 5 object key 1\" value: 10, 30, 219, 2\n"	\
"integer: 23: 10, 30, 219, 2\n"						\
"object key \"root key 5 object key 2\" value: 11, 30, 252, 12\n"	\
"real: 3.141593: 11, 30, 252, 12\n"					\
"object key \"root key emoji\" value: 13, 20, 288, 14\n"		\
"string: \"😂\": 13, 20, 288, 14\n"

static void print_location(char *outbuf, ssize_t *outbufspace,
                           json_t *root, const char *fmt, ...)
{
    va_list ap;
    char buf[1024];
    int line, column, position, length;

    if (*outbufspace <= 0)
        return;

    if (json_get_location(root, &line, &column, &position, &length))
        return;

    va_start(ap, fmt);
    vsnprintf(buf, 1024, fmt, ap);
    va_end(ap);
    strncat(outbuf, buf, *outbufspace);
    *outbufspace -= strlen(buf);

    if (*outbufspace <= 0)
        return;

    sprintf(buf, ": %d, %d, %d, %d\n", line, column, position, length);
    strncat(outbuf, buf, *outbufspace);
    *outbufspace -= strlen(buf);
}

static void parse(char *outbuf, ssize_t *outbufspace, json_t *root)
{
    unsigned int index;
    const char *key;
    json_t *tmp;

    switch(json_typeof(root)) {
    case JSON_OBJECT:
        print_location(outbuf, outbufspace, root, "object");
        json_object_foreach(root, key, tmp) {
            print_location(outbuf, outbufspace, tmp,
                           "object key \"%s\" value", key);
            parse(outbuf, outbufspace, tmp);
        }
        break;
    case JSON_ARRAY:
        print_location(outbuf, outbufspace, root, "array");
        json_array_foreach(root, index, tmp) {
            print_location(outbuf, outbufspace, tmp,
                           "array item %u", index);
            parse(outbuf, outbufspace, tmp);
        }
        break;
    case JSON_STRING:
        print_location(outbuf, outbufspace, root,
                       "string: \"%s\"", json_string_value(root));
        break;
    case JSON_INTEGER:
        print_location(outbuf, outbufspace, root,
                       "integer: %" JSON_INTEGER_FORMAT,
                       json_integer_value(root));
        break;
    case JSON_REAL:
        print_location(outbuf, outbufspace, root,
                       "real: %lf", json_real_value(root));
        break;
    case JSON_TRUE:
        print_location(outbuf, outbufspace, root, "true");
        break;
    case JSON_FALSE:
        print_location(outbuf, outbufspace, root, "false");
        break;
    case JSON_NULL:
        print_location(outbuf, outbufspace, root, "null");
        break;
    }
}

static void run_test(const char *input, const char *output, int flags)
{
    ssize_t bufspace;
    char buf[8192];

    json_t *root = json_loads(input, flags, NULL);
    if (!root)
        fail("loading input failed");
    *buf = '\0';
    bufspace = sizeof(buf) - 1;
    parse(buf, &bufspace, root);
    json_decref(root);

    if (strcmp(buf, output))
        fail_args("output doesn't match:\nexpect:\n%s\ngot:\n%s\n", output, buf);
}

static void run_tests(void)
{
    run_test(INPUT, "", 0);
    run_test(MULTILINE_INPUT, "", 0);
    run_test(INPUT, OUTPUT, JSON_STORE_LOCATION);
    run_test(MULTILINE_INPUT, MULTILINE_OUTPUT, JSON_STORE_LOCATION);
}
