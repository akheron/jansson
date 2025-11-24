/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "utf.hpp"
#include <cstring>
#include <stdexcept>

namespace jansson {
namespace utf {

int encode(int32_t codepoint, char *buffer, size_t *size) {
    if (codepoint < 0x80) {
        buffer[0] = static_cast<char>(codepoint);
        *size = 1;
    } else if (codepoint < 0x800) {
        buffer[0] = static_cast<char>(0xC0 | (codepoint >> 6));
        buffer[1] = static_cast<char>(0x80 | (codepoint & 0x3F));
        *size = 2;
    } else if (codepoint < 0x10000) {
        buffer[0] = static_cast<char>(0xE0 | (codepoint >> 12));
        buffer[1] = static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        buffer[2] = static_cast<char>(0x80 | (codepoint & 0x3F));
        *size = 3;
    } else if (codepoint <= 0x10FFFF) {
        buffer[0] = static_cast<char>(0xF0 | (codepoint >> 18));
        buffer[1] = static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        buffer[2] = static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        buffer[3] = static_cast<char>(0x80 | (codepoint & 0x3F));
        *size = 4;
    } else {
        return -1;
    }
    return 0;
}

size_t check_first(char byte) {
    unsigned char u = static_cast<unsigned char>(byte);
    
    if (u < 0x80) {
        return 1;
    }
    
    if (0x80 <= u && u <= 0xBF) {
        /* second, third or fourth byte of a multi-byte
           sequence, i.e. a "continuation byte" */
        return 0;
    } else if (u == 0xC0 || u == 0xC1) {
        /* overlong encoding of an ASCII character */
        return 0;
    } else if (0xC2 <= u && u <= 0xDF) {
        /* 2-byte sequence */
        return 2;
    } else if (0xE0 <= u && u <= 0xEF) {
        /* 3-byte sequence */
        return 3;
    } else if (0xF0 <= u && u <= 0xF4) {
        /* 4-byte sequence */
        return 4;
    } else {
        /* u >= 0xF5 */
        return 0;
    }
}

size_t check_full(const char *buffer, size_t size, int32_t *codepoint) {
    size_t count = check_first(buffer[0]);
    if (count == 0) {
        return 0;
    }
    
    if (count > size) {
        return 0;
    }
    
    unsigned char u = static_cast<unsigned char>(buffer[0]);
    int32_t value = 0;
    
    if (count == 1) {
        value = u;
    } else if (count == 2) {
        value = u & 0x1F;
    } else if (count == 3) {
        value = u & 0xF;
    } else if (count == 4) {
        value = u & 0x7;
    }
    
    for (size_t i = 1; i < count; ++i) {
        u = static_cast<unsigned char>(buffer[i]);
        
        if (u < 0x80 || u > 0xBF) {
            return 0;
        }
        
        value = (value << 6) + (u & 0x3F);
    }
    
    if (value > 0x10FFFF) {
        return 0;
    }
    
    /* Overlong encodings */
    if ((count == 2 && value < 0x80) ||
        (count == 3 && value < 0x800) ||
        (count == 4 && value < 0x10000)) {
        return 0;
    }
    
    if (codepoint) {
        *codepoint = value;
    }
    
    return count;
}

bool check_string(const char *string, size_t length) {
    size_t i = 0;
    
    while (i < length) {
        size_t count = check_full(&string[i], length - i, nullptr);
        if (count == 0) {
            return false;
        }
        i += count;
    }
    
    return true;
}

bool check_string(const std::string& str) {
    return check_string(str.c_str(), str.length());
}

const char *iterate(const char *buffer, size_t bufsize, int32_t *codepoint) {
    size_t count;
    int32_t value;
    
    if (bufsize == 0) {
        return nullptr;
    }
    
    count = check_first(buffer[0]);
    if (count == 0) {
        return nullptr;
    }
    
    if (count > bufsize) {
        return nullptr;
    }
    
    if (count == 1) {
        value = static_cast<unsigned char>(buffer[0]);
    } else {
        if (!check_full(buffer, count, &value)) {
            return nullptr;
        }
    }
    
    if (codepoint) {
        *codepoint = value;
    }
    
    return buffer + count;
}

size_t char_length(const char *string, size_t length) {
    size_t i = 0;
    size_t count = 0;
    
    while (i < length) {
        size_t n = check_full(&string[i], length - i, nullptr);
        if (n == 0) {
            return 0;
        }
        i += n;
        count++;
    }
    
    return count;
}

size_t char_length(const std::string& str) {
    return char_length(str.c_str(), str.length());
}

} // namespace utf
} // namespace jansson
