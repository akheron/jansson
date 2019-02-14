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

#define OUTPUT							\
"object at line 1 column 1 length 1\n"				\
"object key \"testkey\" value at line 1 column 14 length 1\n"	\
"array at line 1 column 14 length 1\n"				\
"array item 0 at line 1 column 15 length 12\n"			\
"string: \"testvalue1\" at line 1 column 15 length 12\n"	\
"array item 1 at line 1 column 29 length 12\n"			\
"string: \"testvalue2\" at line 1 column 29 length 12\n"

#define MULTILINE_OUTPUT							\
"object at line 1 column 1 length 1\n"						\
"object key \"root key 1\" value at line 2 column 16 length 20\n"		\
"string: \"root key 1 value 1\" at line 2 column 16 length 20\n"		\
"object key \"root key 2\" value at line 3 column 16 length 1\n"		\
"array at line 3 column 16 length 1\n"						\
"array item 0 at line 4 column 3 length 26\n"					\
"string: \"root key 2 array value 1\" at line 4 column 3 length 26\n"		\
"array item 1 at line 5 column 3 length 26\n"					\
"string: \"root key 2 array value 2\" at line 5 column 3 length 26\n"		\
"object key \"root key 3\" value at line 7 column 16 length 1\n"		\
"array at line 7 column 16 length 1\n"						\
"array item 0 at line 7 column 18 length 4\n"					\
"true at line 7 column 18 length 4\n"						\
"array item 1 at line 7 column 24 length 5\n"					\
"false at line 7 column 24 length 5\n"						\
"object key \"root key 4\" value at line 8 column 16 length 4\n"		\
"null at line 8 column 16 length 4\n"						\
"object key \"root key 5\" value at line 9 column 16 length 1\n"		\
"object at line 9 column 16 length 1\n"						\
"object key \"root key 5 object key 1\" value at line 10 column 30 length 2\n"	\
"integer: 23 at line 10 column 30 length 2\n"					\
"object key \"root key 5 object key 2\" value at line 11 column 30 length 12\n"	\
"real: 3.141593 at line 11 column 30 length 12\n"				\
"object key \"root key emoji\" value at line 13 column 20 length 14\n"		\
"string: \"😂\" at line 13 column 20 length 14\n"

static void print_location(char *outbuf, ssize_t *outbufspace,
			   json_t *root, const char *fmt, ...)
{
	va_list ap;
	char buf[1024];
	int line, column, length;

	if (*outbufspace <= 0)
		return;

	if (json_get_location(root, &line, &column, NULL, &length))
		return;

	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);
	strncat(outbuf, buf, *outbufspace);
	*outbufspace -= strlen(buf);

	if (*outbufspace <= 0)
		return;

	sprintf(buf, " at line %d column %d length %d\n", line, column, length);
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
	default:
		print_location(outbuf, outbufspace, root, "unknown");
	}
}

static void run_test(const char *input, const char *output, int store)
{
	ssize_t bufspace;
	char buf[8192];

	json_t *root = json_loads(input, store ? JSON_STORE_LOCATION : 0, NULL);
	if (!root)
		fail("loading input failed");
	*buf = '\0';
	bufspace = sizeof(buf) - 1;
	parse(buf, &bufspace, root);
	json_decref(root);

	if (store && strcmp(buf, output))
		fail_args("output doesn't match:\nexpect:\n%s\ngot:\n%s\n", output, buf);
	else if (!store && strlen(buf))
		fail("got output where none should be\n");
}

static void run_tests(void)
{
	run_test(INPUT, OUTPUT, 0);
	run_test(MULTILINE_INPUT, MULTILINE_OUTPUT, 0);
	run_test(INPUT, OUTPUT, 1);
	run_test(MULTILINE_INPUT, MULTILINE_OUTPUT, 1);
}
