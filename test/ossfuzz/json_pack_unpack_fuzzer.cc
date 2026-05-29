/*
 * json_pack_unpack_fuzzer.cc
 *
 * Fuzz harness for jansson json_unpack() and json_deep_copy() paths.
 * json_unpack() processes a format string + JSON value and extracts
 * typed fields — it involves non-trivial string/type dispatch that
 * is not exercised by the load/dump fuzzer.
 *
 * OSS-Fuzz build: compiled via test/ossfuzz/ossfuzz.sh.
 */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "jansson.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 2)
        return 0;

    /* Use first byte to select format variant, rest as JSON input */
    uint8_t variant = data[0] % 8;
    const char *json_str = (const char *)(data + 1);
    size_t json_len = size - 1;

    json_error_t error;
    json_t *root = json_loadb(json_str, json_len,
                               JSON_DECODE_ANY | JSON_ALLOW_NUL, &error);
    if (root == NULL)
        return 0;

    /* Exercise json_deep_copy on whatever we parsed */
    json_t *copy = json_deep_copy(root);
    if (copy != NULL)
        json_decref(copy);

    /* Exercise json_dumps with various flags */
    const int flag_sets[] = {
        0,
        JSON_COMPACT,
        JSON_ENSURE_ASCII,
        JSON_SORT_KEYS,
        JSON_COMPACT | JSON_ENSURE_ASCII,
    };
    int flags = flag_sets[variant % 5];
    char *dumped = json_dumps(root, flags);
    if (dumped != NULL) {
        /* Re-load the dumped output for round-trip check */
        json_t *reparsed = json_loads(dumped, 0, NULL);
        if (reparsed != NULL)
            json_decref(reparsed);
        free(dumped);
    }

    /* Exercise json_unpack with simple format strings if root is an object */
    if (json_is_object(root)) {
        json_t *val = NULL;
        const char *key = NULL;
        /* Unpack first key-value pair */
        void *iter = json_object_iter(root);
        if (iter != NULL) {
            key = json_object_iter_key(iter);
            val  = json_object_iter_value(iter);
            (void)key;
            (void)val;
        }

        /* json_unpack with "o" format — extract object */
        json_t *out_obj = NULL;
        json_unpack(root, "o", &out_obj);

        /* json_unpack with "{s?o}" — optional key lookup */
        json_t *field = NULL;
        json_unpack(root, "{s?o}", "data", &field);
    }

    /* Exercise json_unpack on array */
    if (json_is_array(root) && json_array_size(root) > 0) {
        json_t *first = NULL;
        json_unpack(root, "[o!]", &first);
    }

    json_decref(root);
    return 0;
}
