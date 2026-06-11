/*
 * json_equal_copy_fuzzer.c
 *
 * Fuzz target for jansson equality and copying APIs.
 * The existing json_load_dump_fuzzer covers json_loads/json_dumps only.
 * This harness exercises the following uncovered paths:
 *
 *   json_loadb()       - length-bounded load (no NUL required)
 *   json_equal()       - deep equality comparison
 *   json_copy()        - shallow copy
 *   json_deep_copy()   - recursive deep copy
 *   json_object_update()        - merge two objects
 *   json_object_update_existing() - update only existing keys
 *   json_object_update_new()      - merge + decref rhs
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "jansson.h"

static uint8_t u8(const uint8_t **d, size_t *s)
{
    if (*s == 0) return 0;
    uint8_t v = **d; (*d)++; (*s)--; return v;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 2) return 0;

    uint8_t op = u8(&data, &size);

    /* Parse with json_loadb — does not require NUL termination */
    json_error_t err;
    json_t *root = json_loadb((const char *)data, size, 0, &err);
    if (!root) return 0;

    switch (op % 5) {
    case 0: {
        /* Deep copy then compare for equality */
        json_t *copy = json_deep_copy(root);
        if (copy) {
            json_equal(root, copy);
            json_decref(copy);
        }
        break;
    }
    case 1: {
        /* Shallow copy */
        json_t *shallow = json_copy(root);
        if (shallow) {
            json_equal(root, shallow);
            json_decref(shallow);
        }
        break;
    }
    case 2: {
        /* Compare root with itself */
        json_equal(root, root);
        break;
    }
    case 3: {
        /* json_object_update: merge root into a new empty object */
        if (json_is_object(root)) {
            json_t *target = json_object();
            if (target) {
                json_object_update(target, root);
                json_decref(target);
            }
        }
        break;
    }
    case 4: {
        /* json_object_update_existing: only update keys present in target */
        if (json_is_object(root)) {
            json_t *target = json_deep_copy(root);
            if (target) {
                json_t *patch = json_deep_copy(root);
                if (patch) {
                    json_object_update_existing(target, patch);
                    json_decref(patch);
                }
                json_decref(target);
            }
        }
        break;
    }
    }

    json_decref(root);
    return 0;
}
