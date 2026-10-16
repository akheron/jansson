#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "util.h"
#include <jansson.h>
#include <stdio.h>
#include <string.h>

static int chaos_pos = 0;
static int chaos_fail = 0;
#define CHAOS_MAX_FAILURE 100

void *chaos_malloc(size_t size) {
    if (chaos_pos == chaos_fail)
        return NULL;

    chaos_pos++;

    return malloc(size);
}

void chaos_free(void *obj) { free(obj); }

/* Test all potential allocation failures. */
#define chaos_loop(condition, code, cleanup)                                             \
    {                                                                                    \
        chaos_pos = chaos_fail = 0;                                                      \
        while (condition) {                                                              \
            if (chaos_fail > CHAOS_MAX_FAILURE)                                          \
                fail("too many chaos failures");                                         \
            code chaos_pos = 0;                                                          \
            chaos_fail++;                                                                \
        }                                                                                \
        cleanup                                                                          \
    }

#define chaos_loop_new_value(json, initcall)                                             \
    chaos_loop(!json, json = initcall;, json_decref(json); json = NULL;)

int test_unpack() {
    int ret = -1;
    int v1;
    int v2;
    json_error_t error;
    json_t *root = json_pack("{s:i, s:i, s:i, s:i}", "n1", 1, "n2", 2, "n3", 3, "n4", 4);

    if (!root)
        return -1;

    if (!json_unpack_ex(root, &error, JSON_STRICT, "{s:i, s:i}", "n1", &v1, "n2", &v2))
        fail("Unexpected success");

    if (json_error_code(&error) != json_error_end_of_input_expected) {
        if (json_error_code(&error) != json_error_out_of_memory)
            fail("Unexpected error code");

        goto out;
    }

    if (strcmp(error.text, "2 object item(s) left unpacked: n3, n4"))
        goto out;

    ret = 0;

out:
    json_decref(root);
    return ret;
}

int dump_chaos_callback(const char *buffer, size_t size, void *data) {
    json_t *obj = json_object();

    (void)buffer;
    (void)size;
    (void)data;

    if (!obj)
        return -1;

    json_decref(obj);

    return 0;
}

static void test_chaos() {
    json_malloc_t orig_malloc;
    json_free_t orig_free;
    json_t *json = NULL;
    json_t *obj = json_object();
    json_t *arr1 = json_array();
    json_t *arr2 = json_array();
    json_t *txt = json_string("test");
    json_t *intnum = json_integer(1);
    json_t *dblnum = json_real(0.5);
    char *dumptxt = NULL;
    json_t *dumpobj = json_pack("{s:[iiis], s:s}", "key1", 1, 2, 3, "txt", "key2", "v2");
    int keyno;

    if (!obj || !arr1 || !arr2 || !txt || !intnum || !dblnum || !dumpobj)
        fail("failed to allocate basic objects");

    json_get_alloc_funcs(&orig_malloc, &orig_free);
    json_set_alloc_funcs(chaos_malloc, chaos_free);

    chaos_loop_new_value(json, json_pack("{s:s}", "key", "value"));
    chaos_loop_new_value(json, json_pack("{s:[]}", "key"));
    chaos_loop_new_value(json, json_pack("[biIf]", 1, 1, (json_int_t)1, 1.0));
    chaos_loop_new_value(json, json_pack("[s*,s*]", "v1", "v2"));
    chaos_loop_new_value(json, json_pack("o", json_incref(txt)));
    chaos_loop_new_value(json, json_pack("O", txt));
    chaos_loop_new_value(json, json_pack("s++", "a", "long string to force realloc",
                                         "another long string to force yet another "
                                         "reallocation of the string because "
                                         "that's what we are testing."));

    chaos_loop(test_unpack(), , );

    chaos_loop(json_dump_callback(dumpobj, dump_chaos_callback, NULL, JSON_INDENT(1)),
               , );
    chaos_loop(json_dump_callback(dumpobj, dump_chaos_callback, NULL,
                                  JSON_INDENT(1) | JSON_SORT_KEYS),
               , );
    chaos_loop(!dumptxt, dumptxt = json_dumps(dumpobj, JSON_COMPACT);, free(dumptxt);
               dumptxt = NULL;);

    chaos_loop_new_value(json, json_copy(obj));
    chaos_loop_new_value(json, json_deep_copy(obj));

    chaos_loop_new_value(json, json_copy(arr1));
    chaos_loop_new_value(json, json_deep_copy(arr1));

    chaos_loop_new_value(json, json_copy(txt));
    chaos_loop_new_value(json, json_copy(intnum));
    chaos_loop_new_value(json, json_copy(dblnum));

#define JSON_LOAD_TXT "{\"n\":[1,2,3,4,5,6,7,8,9,10]}"
    chaos_loop_new_value(json, json_loads(JSON_LOAD_TXT, 0, NULL));
    chaos_loop_new_value(json, json_loadb(JSON_LOAD_TXT, strlen(JSON_LOAD_TXT), 0, NULL));

    chaos_loop_new_value(json, json_sprintf("%s", "string"));

    for (keyno = 0; keyno < 100; ++keyno) {
#if !defined(_MSC_VER) || _MSC_VER >= 1900
        /* Skip this test on old Windows compilers. */
        char testkey[10];

        snprintf(testkey, sizeof(testkey), "test%d", keyno);
        chaos_loop(json_object_set_new_nocheck(obj, testkey, json_object()), , );
#endif
        chaos_loop(json_array_append_new(arr1, json_null()), , );
        chaos_loop(json_array_insert_new(arr2, 0, json_null()), , );
    }

    chaos_loop(json_array_extend(arr1, arr2), , );
    chaos_loop(json_string_set_nocheck(txt, "test"), , );

    json_set_alloc_funcs(orig_malloc, orig_free);
    json_decref(obj);
    json_decref(arr1);
    json_decref(arr2);
    json_decref(txt);
    json_decref(intnum);
    json_decref(dblnum);
    json_decref(dumpobj);
}

/*
 * Fail exactly one allocation at a time and let all later allocations
 * succeed.  Unlike chaos_loop() above, where every allocation fails once
 * the failing index is reached, a one-shot failure can leave the lexer
 * with a partially saved token while the rest of the input is consumed
 * normally.  json_loads() must then either fail or return the exact value
 * of the input; it used to silently return a wrong value (a character
 * that could not be saved went missing from the token text) or corrupt
 * the heap (a JSON string whose closing quotation mark went missing made
 * lex_scan_string() decode past the end of the token buffer).
 */
static int once_pos = 0;
static int once_fail = -1;

static void *once_malloc(size_t size) {
    if (once_pos++ == once_fail)
        return NULL;

    return malloc(size);
}

static void once_free(void *obj) { free(obj); }

#define ONCE_MAX_FAILURE 60

static void once_loop_load(const char *text) {
    json_t *expected = json_loads(text, 0, NULL);
    json_malloc_t orig_malloc;
    json_free_t orig_free;
    int fail_index;

    if (!expected) {
        failhdr;
        fprintf(stderr, "failed to parse the test input: %s\n", text);
        exit(1);
    }

    json_get_alloc_funcs(&orig_malloc, &orig_free);

    for (fail_index = 0; fail_index <= ONCE_MAX_FAILURE; fail_index++) {
        json_t *json;

        once_pos = 0;
        once_fail = fail_index;
        json_set_alloc_funcs(once_malloc, once_free);
        json = json_loads(text, 0, NULL);
        json_set_alloc_funcs(orig_malloc, orig_free);
        once_fail = -1;

        if (json) {
            if (!json_equal(json, expected))
                fail("json_loads() returned a wrong value with an allocation failing");
            json_decref(json);
        }
    }

    json_decref(expected);
}

static void test_load_alloc_failure() {
    char text[128];
    int len;

    /* JSON strings of increasing length.  The token text buffer grows at
     * 16, 32, ... bytes, so with some lengths the character being saved
     * when the buffer grows is the closing quotation mark (body lengths
     * 14 and 30 here), and with others it is a character in the middle of
     * the string. */
    for (len = 1; len <= 40; len++) {
        text[0] = '[';
        text[1] = '"';
        memset(&text[2], 'a', len);
        text[2 + len] = '"';
        text[3 + len] = ']';
        text[4 + len] = '\0';
        once_loop_load(text);
    }

    /* The same with escape sequences and a surrogate pair in the middle,
     * to exercise every branch of the string decoder. */
    once_loop_load("[\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\"]");
    once_loop_load("[\"\\uD83D\\uDE00xy\"]");
    once_loop_load("[\"aaaaaaaaaaaa\\t\"]");

    /* Integers of increasing length: the token text buffer grows inside
     * the number at some lengths (16 and up here), and a dropped digit
     * used to be accepted as a different number. */
    for (len = 1; len <= 18; len++) {
        text[0] = '[';
        memset(&text[1], '1', len);
        text[1 + len] = ']';
        text[2 + len] = '\0';
        once_loop_load(text);
    }

    /* Real numbers and exponents of increasing length. */
    for (len = 1; len <= 15; len++) {
        memcpy(text, "[1.", 3);
        memset(&text[3], '1', len);
        text[3 + len] = ']';
        text[4 + len] = '\0';
        once_loop_load(text);
    }

    for (len = 1; len <= 15; len++) {
        memcpy(text, "[1", 2);
        memset(&text[2], '1', len);
        memcpy(&text[2 + len], ".1e+1]", 7);
        once_loop_load(text);
    }

    for (len = 1; len <= 15; len++) {
        memcpy(text, "[1.", 3);
        memset(&text[3], '1', len);
        memcpy(&text[3 + len], "E-1]", 5);
        once_loop_load(text);
    }
}

static void run_tests() {
    test_chaos();
    test_load_alloc_failure();
}
