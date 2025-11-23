/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef JANSSON_HPP
#define JANSSON_HPP

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

#include "jansson_config.hpp"

namespace jansson {

/* Version constants */
constexpr int MAJOR_VERSION = 2;
constexpr int MINOR_VERSION = 14;
constexpr int MICRO_VERSION = 1;

/* Version as a string - avoid macro conflicts */
constexpr const char* JANSSON_VERSION_STR = "2.14.1";

/* Version as a 3-byte hex number, e.g. 0x010201 == 1.2.1. Use this
   for numeric comparisons, e.g. #if VERSION_HEX >= ... */
constexpr int VERSION_HEX = ((MAJOR_VERSION << 16) | (MINOR_VERSION << 8) | (MICRO_VERSION << 0));

/* If __atomic or __sync builtins are available the library is thread
 * safe for all read-only functions plus reference counting. */
#if JSON_HAVE_ATOMIC_BUILTINS || JSON_HAVE_SYNC_BUILTINS
constexpr bool THREAD_SAFE_REFCOUNT = true;
#else
constexpr bool THREAD_SAFE_REFCOUNT = false;
#endif

/* Attributes */
#if defined(__GNUC__) || defined(__clang__)
#define JANSSON_ATTRS(x) __attribute__(x)
#else
#define JANSSON_ATTRS(x)
#endif

/* Forward declarations */
class json_t;

/* Type enumeration */
enum class json_type {
    OBJECT,
    ARRAY,
    STRING,
    INTEGER,
    REAL,
    TRUE,
    FALSE,
    NULL_TYPE
};

/* JSON value class */
class json_t {
public:
    json_type type;
    volatile size_t refcount;
    
    json_t(json_type t) : type(t), refcount(1) {}
    virtual ~json_t() = default;
};

/* Reference counting functions */
inline json_t* json_incref(json_t* json);
inline void json_decref(json_t* json);

/* Smart pointer for automatic memory management */
using json_ptr = std::unique_ptr<json_t, decltype(&json_decref)>;

inline json_ptr make_json_ptr(json_t* json) {
    return json_ptr(json, json_decref);
}

/* Integer type configuration */
#ifndef JANSSON_USING_CMAKE /* disabled if using cmake */
#if JSON_INTEGER_IS_LONG_LONG
#ifdef _WIN32
constexpr const char* INTEGER_FORMAT = "I64d";
using json_int_t = long long;
#else
constexpr const char* INTEGER_FORMAT = "lld";
using json_int_t = long long;
#endif /* JSON_INTEGER_IS_LONG_LONG */
#else
constexpr const char* INTEGER_FORMAT = "ld";
using json_int_t = long;
#endif /* JSON_INTEGER_IS_LONG_LONG */
#endif

/* Type checking functions */
inline json_type json_typeof(const json_t* json) {
    return json ? json->type : json_type::NULL_TYPE;
}

inline bool json_is_object(const json_t* json) {
    return json && json_typeof(json) == json_type::OBJECT;
}

inline bool json_is_array(const json_t* json) {
    return json && json_typeof(json) == json_type::ARRAY;
}

inline bool json_is_string(const json_t* json) {
    return json && json_typeof(json) == json_type::STRING;
}

inline bool json_is_integer(const json_t* json) {
    return json && json_typeof(json) == json_type::INTEGER;
}

inline bool json_is_real(const json_t* json) {
    return json && json_typeof(json) == json_type::REAL;
}

inline bool json_is_number(const json_t* json) {
    return json_is_integer(json) || json_is_real(json);
}

inline bool json_is_true(const json_t* json) {
    return json && json_typeof(json) == json_type::TRUE;
}

inline bool json_is_false(const json_t* json) {
    return json && json_typeof(json) == json_type::FALSE;
}

inline bool json_is_boolean(const json_t* json) {
    return json_is_true(json) || json_is_false(json);
}

inline bool json_is_null(const json_t* json) {
    return json && json_typeof(json) == json_type::NULL_TYPE;
}

/* Reference counting */
#if JSON_HAVE_ATOMIC_BUILTINS
inline size_t json_incref_internal(json_t* json) {
    return __atomic_add_fetch(&json->refcount, 1, __ATOMIC_ACQUIRE);
}

inline size_t json_decref_internal(json_t* json) {
    return __atomic_sub_fetch(&json->refcount, 1, __ATOMIC_RELEASE);
}
#elif JSON_HAVE_SYNC_BUILTINS
inline size_t json_incref_internal(json_t* json) {
    return __sync_add_and_fetch(&json->refcount, 1);
}

inline size_t json_decref_internal(json_t* json) {
    return __sync_sub_and_fetch(&json->refcount, 1);
}
#else
inline size_t json_incref_internal(json_t* json) {
    return ++json->refcount;
}

inline size_t json_decref_internal(json_t* json) {
    return --json->refcount;
}
#endif

inline json_t* json_incref(json_t* json) {
    if (json && json->refcount != static_cast<size_t>(-1)) {
        json_incref_internal(json);
    }
    return json;
}

void json_delete(json_t* json);

inline void json_decref(json_t* json) {
    if (json && json->refcount != static_cast<size_t>(-1) && 
        json_decref_internal(json) == 0) {
        json_delete(json);
    }
}

#if defined(__GNUC__) || defined(__clang__)
inline void json_decrefp(json_t** json) {
    if (json) {
        json_decref(*json);
        *json = nullptr;
    }
}

using json_auto_t = json_t __attribute__((cleanup(json_decrefp)));
#endif

/* Error handling */
constexpr int JSON_ERROR_TEXT_LENGTH = 160;
constexpr int JSON_ERROR_SOURCE_LENGTH = 80;

struct json_error_t {
    int line;
    int column;
    int position;
    char source[JSON_ERROR_SOURCE_LENGTH];
    char text[JSON_ERROR_TEXT_LENGTH];
    
    json_error_t() : line(0), column(0), position(0) {
        source[0] = '\0';
        text[0] = '\0';
    }
};

enum class json_error_code {
    unknown,
    out_of_memory,
    stack_overflow,
    cannot_open_file,
    invalid_argument,
    invalid_utf8,
    premature_end_of_input,
    end_of_input_expected,
    invalid_syntax,
    invalid_format,
    wrong_type,
    null_character,
    null_value,
    null_byte_in_key,
    duplicate_key,
    numeric_overflow,
    item_not_found,
    index_out_of_range
};

inline enum json_error_code get_json_error_code(const json_error_t* e) {
    return static_cast<enum json_error_code>(e->text[JSON_ERROR_TEXT_LENGTH - 1]);
}

/* JSON creation functions */
json_t* json_object();
json_t* json_array();
json_t* json_string(const std::string& value);
json_t* json_stringn(const std::string& value, size_t len);
json_t* json_string_nocheck(const std::string& value);
json_t* json_stringn_nocheck(const std::string& value, size_t len);
json_t* json_integer(json_int_t value);
json_t* json_real(double value);
json_t* json_true();
json_t* json_false();
inline json_t* json_boolean(bool val) {
    return val ? json_true() : json_false();
}
json_t* json_null();

/* Object manipulation */
void json_object_seed(size_t seed);
size_t json_object_size(const json_t* object);
json_t* json_object_get(const json_t* object, const std::string& key);
json_t* json_object_getn(const json_t* object, const std::string& key, size_t key_len);
int json_object_set_new(json_t* object, const std::string& key, json_t* value);
int json_object_setn_new(json_t* object, const std::string& key, size_t key_len, json_t* value);
int json_object_set_new_nocheck(json_t* object, const std::string& key, json_t* value);
int json_object_setn_new_nocheck(json_t* object, const std::string& key, size_t key_len, json_t* value);
int json_object_del(json_t* object, const std::string& key);
int json_object_deln(json_t* object, const std::string& key, size_t key_len);
int json_object_clear(json_t* object);
int json_object_update(json_t* object, json_t* other);
int json_object_update_existing(json_t* object, json_t* other);
int json_object_update_missing(json_t* object, json_t* other);
int json_object_update_recursive(json_t* object, json_t* other);

/* Object iteration */
void* json_object_iter(json_t* object);
void* json_object_iter_at(json_t* object, const std::string& key);
void* json_object_key_to_iter(const std::string& key);
void* json_object_iter_next(json_t* object, void* iter);
const std::string json_object_iter_key(void* iter);
size_t json_object_iter_key_len(void* iter);
json_t* json_object_iter_value(void* iter);
int json_object_iter_set_new(json_t* object, void* iter, json_t* value);

/* Array manipulation */
size_t json_array_size(const json_t* array);
json_t* json_array_get(const json_t* array, size_t index);
int json_array_set_new(json_t* array, size_t index, json_t* value);
int json_array_append_new(json_t* array, json_t* value);
int json_array_insert_new(json_t* array, size_t index, json_t* value);
int json_array_remove(json_t* array, size_t index);
int json_array_clear(json_t* array);
int json_array_extend(json_t* array, json_t* other);

/* Value access */
std::string json_string_value(const json_t* string);
size_t json_string_length(const json_t* string);
json_int_t json_integer_value(const json_t* integer);
double json_real_value(const json_t* real);
double json_number_value(const json_t* json);

/* Value modification */
int json_string_set(json_t* string, const std::string& value);
int json_string_setn(json_t* string, const std::string& value, size_t len);
int json_string_set_nocheck(json_t* string, const std::string& value);
int json_string_setn_nocheck(json_t* string, const std::string& value, size_t len);
int json_integer_set(json_t* integer, json_int_t value);
int json_real_set(json_t* real, double value);

/* Pack/unpack functions */
json_t* json_pack(const std::string& fmt, ...);
json_t* json_pack_ex(json_error_t* error, size_t flags, const std::string& fmt, ...);
json_t* json_vpack_ex(json_error_t* error, size_t flags, const std::string& fmt, std::va_list ap);

constexpr int JSON_VALIDATE_ONLY = 0x1;
constexpr int JSON_STRICT = 0x2;

int json_unpack(json_t* root, const std::string& fmt, ...);
int json_unpack_ex(json_t* root, json_error_t* error, size_t flags, const std::string& fmt, ...);
int json_vunpack_ex(json_t* root, json_error_t* error, size_t flags, const std::string& fmt, std::va_list ap);

/* String formatting */
json_t* json_sprintf(const std::string& fmt, ...);
json_t* json_vsprintf(const std::string& fmt, std::va_list ap);

/* Equality */
bool json_equal(const json_t* value1, const json_t* value2);

/* Copying */
json_t* json_copy(json_t* value);
json_t* json_deep_copy(const json_t* value);

/* Loading/Decoding */
constexpr int JSON_REJECT_DUPLICATES = 0x1;
constexpr int JSON_DISABLE_EOF_CHECK = 0x2;
constexpr int JSON_DECODE_ANY = 0x4;
constexpr int JSON_DECODE_INT_AS_REAL = 0x8;
constexpr int JSON_ALLOW_NUL = 0x10;

using json_load_callback_t = size_t(*)(void* buffer, size_t buflen, void* data);

json_t* json_loads(const std::string& input, size_t flags, json_error_t* error);
json_t* json_loadb(const std::string& buffer, size_t buflen, size_t flags, json_error_t* error);
json_t* json_loadf(std::FILE* input, size_t flags, json_error_t* error);
json_t* json_loadfd(int input, size_t flags, json_error_t* error);
json_t* json_load_file(const std::string& path, size_t flags, json_error_t* error);
json_t* json_load_callback(json_load_callback_t callback, void* data, size_t flags, json_error_t* error);

/* Dumping/Encoding */
constexpr int JSON_MAX_INDENT = 0x1F;
inline int JSON_INDENT(int n) { return (n) & JSON_MAX_INDENT; }
constexpr int JSON_COMPACT = 0x20;
constexpr int JSON_ENSURE_ASCII = 0x40;
constexpr int JSON_SORT_KEYS = 0x80;
constexpr int JSON_PRESERVE_ORDER = 0x100;
constexpr int JSON_ENCODE_ANY = 0x200;
constexpr int JSON_ESCAPE_SLASH = 0x400;
inline int JSON_REAL_PRECISION(int n) { return ((n) & 0x1F) << 11; }
constexpr int JSON_EMBED = 0x10000;

using json_dump_callback_t = int(*)(const char* buffer, size_t size, void* data);

std::string json_dumps(const json_t* json, size_t flags);
size_t json_dumpb(const json_t* json, char* buffer, size_t size, size_t flags);
int json_dumpf(const json_t* json, std::FILE* output, size_t flags);
int json_dumpfd(const json_t* json, int output, size_t flags);
int json_dump_file(const json_t* json, const std::string& path, size_t flags);
int json_dump_callback(const json_t* json, json_dump_callback_t callback, void* data, size_t flags);

/* Memory allocation */
using json_malloc_t = void*(*)(size_t);
using json_realloc_t = void*(*)(void*, size_t);
using json_free_t = void(*)(void*);

void json_set_alloc_funcs(json_malloc_t malloc_fn, json_free_t free_fn);
void json_get_alloc_funcs(json_malloc_t* malloc_fn, json_free_t* free_fn);
void json_set_alloc_funcs2(json_malloc_t malloc_fn, json_realloc_t realloc_fn, json_free_t free_fn);
void json_get_alloc_funcs2(json_malloc_t* malloc_fn, json_realloc_t* realloc_fn, json_free_t* free_fn);

/* Version checking */
const std::string jansson_version_str();
int jansson_version_cmp(int major, int minor, int micro);

/* Convenience functions */
template<typename T>
inline bool json_is_type(const json_t* json, json_type type) {
    return json && json_typeof(json) == type;
}

/* Inline wrapper functions */
inline int json_object_set(json_t* object, const std::string& key, json_t* value) {
    return json_object_set_new(object, key, json_incref(value));
}

inline int json_object_setn(json_t* object, const std::string& key, size_t key_len, json_t* value) {
    return json_object_setn_new(object, key, key_len, json_incref(value));
}

inline int json_object_set_nocheck(json_t* object, const std::string& key, json_t* value) {
    return json_object_set_new_nocheck(object, key, json_incref(value));
}

inline int json_object_setn_nocheck(json_t* object, const std::string& key, size_t key_len, json_t* value) {
    return json_object_setn_new_nocheck(object, key, key_len, json_incref(value));
}

inline int json_object_iter_set(json_t* object, void* iter, json_t* value) {
    return json_object_iter_set_new(object, iter, json_incref(value));
}

inline int json_object_update_new(json_t* object, json_t* other) {
    int ret = json_object_update(object, other);
    json_decref(other);
    return ret;
}

inline int json_object_update_existing_new(json_t* object, json_t* other) {
    int ret = json_object_update_existing(object, other);
    json_decref(other);
    return ret;
}

inline int json_object_update_missing_new(json_t* object, json_t* other) {
    int ret = json_object_update_missing(object, other);
    json_decref(other);
    return ret;
}

inline int json_array_set(json_t* array, size_t ind, json_t* value) {
    return json_array_set_new(array, ind, json_incref(value));
}

inline int json_array_append(json_t* array, json_t* value) {
    return json_array_append_new(array, json_incref(value));
}

inline int json_array_insert(json_t* array, size_t ind, json_t* value) {
    return json_array_insert_new(array, ind, json_incref(value));
}

} // namespace jansson

#endif // JANSSON_HPP
