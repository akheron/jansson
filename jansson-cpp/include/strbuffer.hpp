/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef STRBUFFER_HPP
#define STRBUFFER_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>

namespace jansson {

class strbuffer {
private:
    std::string data_;
    
public:
    // Constructors
    strbuffer() = default;
    strbuffer(const strbuffer&) = default;
    strbuffer(strbuffer&&) noexcept = default;
    strbuffer& operator=(const strbuffer&) = default;
    strbuffer& operator=(strbuffer&&) noexcept = default;
    ~strbuffer() = default;
    
    // Initialize with empty string (equivalent to strbuffer_init)
    void init() {
        data_.clear();
        data_.reserve(16); // Minimum size from original implementation
    }
    
    // Clear the buffer (equivalent to strbuffer_clear)
    void clear() {
        data_.clear();
    }
    
    // Get const pointer to string data (equivalent to strbuffer_value)
    const char* value() const noexcept {
        return data_.c_str();
    }
    
    // Get reference to internal string
    const std::string& str() const noexcept {
        return data_;
    }
    
    // Steal the value (equivalent to strbuffer_steal_value)
    // Note: This leaves the buffer empty
    char* steal_value() {
        if (data_.empty()) {
            return nullptr;
        }
        char* result = new char[data_.length() + 1];
        std::copy(data_.begin(), data_.end(), result);
        result[data_.length()] = '\0';
        data_.clear();
        return result;
    }
    
    // Append a single byte (equivalent to strbuffer_append_byte)
    bool append_byte(char byte) {
        data_.push_back(byte);
        return true;
    }
    
    // Append multiple bytes (equivalent to strbuffer_append_bytes)
    bool append_bytes(const char* bytes, size_t size) {
        if (!bytes) return false;
        try {
            data_.append(bytes, size);
            return true;
        } catch (const std::bad_alloc&) {
            return false;
        }
    }
    
    // Append a C-style string
    bool append_string(const char* str) {
        if (!str) return false;
        try {
            data_.append(str);
            return true;
        } catch (const std::bad_alloc&) {
            return false;
        }
    }
    
    // Pop a character from the end (equivalent to strbuffer_pop)
    char pop() {
        if (data_.empty()) {
            return '\0';
        }
        char c = data_.back();
        data_.pop_back();
        return c;
    }
    
    // Get current length
    size_t length() const noexcept {
        return data_.length();
    }
    
    // Get current capacity
    size_t capacity() const noexcept {
        return data_.capacity();
    }
    
    // Check if empty
    bool empty() const noexcept {
        return data_.empty();
    }
    
    // Reserve capacity
    void reserve(size_t new_cap) {
        data_.reserve(new_cap);
    }
    
    // Direct member access for compatibility (for existing code that uses strbuff.value, strbuff.length, strbuff.size)
    char* compat_value;     // pointer to string data
    size_t compat_length;   // bytes used
    size_t compat_size;     // bytes allocated
    
    // Initialize compatibility members
    void init_compat() {
        data_.clear();
        data_.reserve(16); // Minimum size from original implementation
        compat_value = const_cast<char*>(data_.c_str());
        compat_length = 0;
        compat_size = data_.capacity();
    }
    
    // Update compatibility members
    void update_compat() {
        compat_value = const_cast<char*>(data_.c_str());
        compat_length = data_.length();
        compat_size = data_.capacity();
    }
};

} // namespace jansson

// C-compatible typedef for backward compatibility
typedef jansson::strbuffer strbuffer_t;

// C-compatible wrapper functions for backward compatibility
inline int strbuffer_init(strbuffer_t *strbuff) {
    if (!strbuff) return -1;
    strbuff->init_compat();
    return 0;
}

inline void strbuffer_close(strbuffer_t *strbuff) {
    if (strbuff) {
        strbuff->clear();
        strbuff->compat_value = nullptr;
        strbuff->compat_length = 0;
        strbuff->compat_size = 0;
    }
}

inline void strbuffer_clear(strbuffer_t *strbuff) {
    if (strbuff) {
        strbuff->clear();
        strbuff->update_compat();
    }
}

inline const char *strbuffer_value(const strbuffer_t *strbuff) {
    return strbuff ? strbuff->compat_value : nullptr;
}

inline char *strbuffer_steal_value(strbuffer_t *strbuff) {
    if (!strbuff) return nullptr;
    char* result = strbuff->steal_value();
    strbuff->compat_value = nullptr;
    strbuff->compat_length = 0;
    strbuff->compat_size = 0;
    return result;
}

inline int strbuffer_append_byte(strbuffer_t *strbuff, char byte) {
    if (!strbuff || !strbuff->append_byte(byte)) return -1;
    strbuff->update_compat();
    return 0;
}

inline int strbuffer_append_bytes(strbuffer_t *strbuff, const char *data, size_t size) {
    if (!strbuff || !strbuff->append_bytes(data, size)) return -1;
    strbuff->update_compat();
    return 0;
}

inline char strbuffer_pop(strbuffer_t *strbuff) {
    if (!strbuff) return '\0';
    char c = strbuff->pop();
    strbuff->update_compat();
    return c;
}

#endif // STRBUFFER_HPP
