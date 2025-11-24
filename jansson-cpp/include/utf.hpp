/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef UTF_HPP
#define UTF_HPP

#include <cstddef>
#include <cstdint>
#include <string>

namespace jansson {
namespace utf {

// Encode a Unicode codepoint to UTF-8
// Returns 0 on success, -1 on error
// buffer must be at least 4 bytes
int encode(int32_t codepoint, char *buffer, size_t *size);

// Check if a byte is a valid first byte of a UTF-8 sequence
// Returns the sequence length (1-4) or 0 for invalid
size_t check_first(char byte);

// Check if a full UTF-8 sequence is valid
// Returns the sequence length (1-4) or 0 for invalid
// If valid and codepoint is not null, stores the codepoint
size_t check_full(const char *buffer, size_t size, int32_t *codepoint);

// Check if a string is valid UTF-8
bool check_string(const char *string, size_t length);

// Check if a string is valid UTF-8 (C++ string overload)
bool check_string(const std::string& str);

// Iterate to the next UTF-8 character
// Returns pointer to next character or nullptr on error
// If codepoint is not null, stores the current codepoint
const char *iterate(const char *buffer, size_t bufsize, int32_t *codepoint);

// Get the length of a UTF-8 string in characters (not bytes)
size_t char_length(const char *string, size_t length);

// Get the length of a UTF-8 string in characters (C++ string overload)
size_t char_length(const std::string& str);

} // namespace utf
} // namespace jansson

// C++ friendly aliases
namespace jansson {
namespace utf {

inline int encode(int32_t codepoint, std::string& buffer) {
    char temp[4];
    size_t size;
    if (encode(codepoint, temp, &size) != 0) {
        return -1;
    }
    buffer.assign(temp, size);
    return 0;
}

inline std::string encode(int32_t codepoint) {
    std::string result;
    encode(codepoint, result);
    return result;
}

} // namespace utf
} // namespace jansson

// Legacy C API compatibility (deprecated, use namespace functions instead)
extern "C" {

inline int utf8_encode(int32_t codepoint, char *buffer, size_t *size) {
    return jansson::utf::encode(codepoint, buffer, size);
}

inline size_t utf8_check_first(char byte) {
    return jansson::utf::check_first(byte);
}

inline size_t utf8_check_full(const char *buffer, size_t size, int32_t *codepoint) {
    return jansson::utf::check_full(buffer, size, codepoint);
}

inline int utf8_check_string(const char *string, size_t length) {
    return jansson::utf::check_string(string, length) ? 1 : 0;
}

inline const char *utf8_iterate(const char *buffer, size_t bufsize, int32_t *codepoint) {
    return jansson::utf::iterate(buffer, bufsize, codepoint);
}

inline size_t utf8_char_length(const char *string, size_t length) {
    return jansson::utf::char_length(string, length);
}

} // extern "C"

#endif // UTF_HPP
