#!/bin/sh

# This test checks that libjansson.so exports the correct symbols.

# The list of symbols that the shared object should export
sort >$test_log/exports <<EOF
json_delete
json_true
json_false
json_null
json_string
json_string_nocheck
json_string_value
json_string_set
json_string_set_nocheck
json_integer
json_integer_value
json_integer_set
json_real
json_real_value
json_real_set
json_number_value
json_array
json_array_size
json_array_get
json_array_set_new
json_array_append_new
json_array_insert_new
json_array_remove
json_array_clear
json_array_extend
json_object
json_object_size
json_object_get
json_object_set_new
json_object_set_new_nocheck
json_object_del
json_object_clear
json_object_update
json_object_update_existing
json_object_update_missing
json_object_iter
json_object_iter_at
json_object_iter_next
json_object_iter_key
json_object_iter_value
json_object_iter_set_new
json_object_key_to_iter
json_dumps
json_dumpf
json_dump_file
json_dump_callback
json_loads
json_loadf
json_load_file
json_loadb
json_equal
json_copy
json_deep_copy
json_pack
json_pack_ex
json_vpack_ex
json_unpack
json_unpack_ex
json_vunpack_ex
json_set_alloc_funcs
EOF

# The list of functions are not exported in the library because they
# are macros or static inline functions. This is only the make the
# list complete, there are not used by the test.
sort >$test_log/macros_or_inline <<EOF
json_typeof
json_incref
json_decref
json_is_object
json_is_object
json_is_array
json_is_string
json_is_integer
json_is_real
json_is_true
json_is_false
json_is_null
json_is_number
json_is_boolean
json_array_set
json_array_append
json_array_insert
json_object_set
json_object_set_nocheck
EOF

SOFILE="../src/.libs/libjansson.so"

nm -D $SOFILE >/dev/null >$test_log/symbols 2>/dev/null \
    || exit 77  # Skip if "nm -D" doesn't seem to work

grep ' T ' $test_log/symbols | cut -d' ' -f3 | sort >$test_log/output

if ! cmp -s $test_log/exports $test_log/output; then
    diff -u $test_log/exports $test_log/output >&2
    exit 1
fi
