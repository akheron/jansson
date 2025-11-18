## **1. Project Assessment**

Before starting, analyze the repository:

* Jansson is a C library for JSON parsing and manipulation.
* It has:

  * **Header files** (`.h`)
  * **Source files** (`.c`)
  * **CMakeLists.txt** and `configure` build files
* Core features:

  * JSON objects are represented as `struct json_t`
  * Functions for manipulation (create, read, write JSON)
* Dependencies: C standard library, `stdint.h`, `stdio.h`, `stdlib.h`, `string.h`

**Goal:** Convert this to idiomatic C++ while keeping API compatibility if possible.

---

## **2. Directory and File Structure**

**Old structure (simplified):**

```
jansson/
  src/
    json.c
    dump.c
    load.c
    ...
  include/
    jansson.h
    version.h
  CMakeLists.txt
```

**New structure:**

```
jansson-cpp/
  include/
    jansson/          # Namespace folder
      json.hpp
      dump.hpp
      load.hpp
      version.hpp
  src/
    json.cpp
    dump.cpp
    load.cpp
    ...
  CMakeLists.txt
```

**Changes:**

* All `.h` → `.hpp`
* Headers go inside `include/jansson/`
* Source files `.c` → `.cpp`
* Use **namespaces** (`namespace jansson`) to encapsulate code
* Consider `internal/` folder for private helpers

---

## **3. Updating CMake**

Original CMake compiles `.c` files:

```cmake
add_library(jansson STATIC
  src/json.c
  src/dump.c
  src/load.c
)
target_include_directories(jansson PUBLIC include)
```

**Updated CMake for C++:**

```cmake
cmake_minimum_required(VERSION 3.16)
project(jansson_cpp VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(jansson_cpp STATIC
    src/json.cpp
    src/dump.cpp
    src/load.cpp
)

target_include_directories(jansson_cpp PUBLIC include)
```

**Notes:**

* Use `CXX` language instead of C
* Enforce at least C++17 for `std::string_view`, smart pointers, structured bindings

---

## **4. File Conversion Strategy**

### **Header files**

* Change `.h` → `.hpp`
* Wrap all declarations in **namespace**:

```cpp
#ifndef JANSSON_JSON_HPP
#define JANSSON_JSON_HPP

#include <string>
#include <vector>
#include <memory>

namespace jansson {

class Json {
public:
    Json();
    ~Json();

    void parse(const std::string& jsonText);
    std::string dump() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl; // PIMPL for internal C structures
};

} // namespace jansson

#endif // JANSSON_JSON_HPP
```

**Notes:**

* Use **classes instead of structs** for encapsulation
* Apply **PIMPL** idiom to hide internal C struct details
* Replace `char*` and raw pointers with `std::string` or `std::vector`
* Replace `malloc/free` with **smart pointers** or `std::vector`

---

### **Source files**

* `.c` → `.cpp`
* Include headers with `#include "jansson/json.hpp"`
* Convert `struct json_t` to a **class `Json`**:

```cpp
struct Json::Impl {
    json_t* root; // original C pointer
};

Json::Json() : pImpl(std::make_unique<Impl>()) {
    pImpl->root = nullptr;
}

Json::~Json() {
    if (pImpl->root) {
        json_decref(pImpl->root); // original C cleanup
    }
}

void Json::parse(const std::string& jsonText) {
    json_error_t error;
    pImpl->root = json_loads(jsonText.c_str(), 0, &error);
    if (!pImpl->root) {
        throw std::runtime_error("Failed to parse JSON: " + std::to_string(error.line));
    }
}
```

**Notes:**

* Encapsulate raw pointers
* Throw exceptions instead of returning error codes
* Use **RAII** for memory management

---

## **5. Convert C APIs to C++ APIs**

Example: Original C function:

```c
json_t *json_object();
void json_object_set_new(json_t *object, const char *key, json_t *value);
```

**C++ version:**

```cpp
class JsonObject : public Json {
public:
    void set(const std::string& key, const Json& value);
};
```

* Use **overloaded methods** instead of multiple functions
* Optional: `operator[]` for convenient access
* Use `std::string` instead of `const char*`

---

## **6. Modern C++ Features to Include**

* **RAII**: manage memory automatically
* **Smart pointers**: `std::unique_ptr`, `std::shared_ptr`
* **std::string / std::vector** instead of C strings and arrays
* **Exceptions** instead of error codes
* **Namespaces** to prevent global pollution
* **PIMPL idiom** to hide internal C implementation
* **Operator overloading** for `[]`, `==`, `!=` on Json objects
* **Move semantics** to improve performance

---

## **7. Optional Improvements**

* Add **C++ iterators** for JSON arrays
* Add **template functions** for type-safe access:

```cpp
template<typename T>
T get(const std::string& key) const;
```

* Add **conversion operators**:

```cpp
operator std::string() const;
operator int() const;
```

* Unit tests with **Catch2** or **Google Test** instead of plain C tests
* Consider splitting the library into **JsonParser**, **JsonWriter**, and **JsonValue** classes

---

## **8. Step-by-Step Conversion Plan**

1. **Setup project structure**

   * Create `include/jansson/` and `src/`
   * Copy `.c` and `.h` files into `src` and `include` respectively
2. **Rename files**

   * `.c` → `.cpp`
   * `.h` → `.hpp`
3. **Update CMake**

   * Set `LANGUAGES CXX`
   * Replace `.c` files with `.cpp`
   * Set `CXX_STANDARD` to 17 or 20
4. **Add namespace**

   * Wrap all headers in `namespace jansson { ... }`
5. **Convert structs to classes**

   * Start with `json_t` → `class Json`
   * Use **PIMPL idiom** for encapsulating C structs
6. **Replace C types with C++ types**

   * `char*` → `std::string`
   * Arrays → `std::vector`
7. **Replace manual memory management**

   * `malloc/free` → smart pointers or RAII
8. **Wrap C API functions into class methods**

   * `json_object_set_new(obj, key, val)` → `obj.set(key, val)`
9. **Introduce exceptions**

   * Replace error codes from `json_loads`, `json_dump` etc.
10. **Add iterators, operator overloads, and templates**
11. **Write unit tests using C++ testing framework**
12. **Build & validate**

* Ensure CMake builds `.lib` or `.a`
* Run tests for correctness

---

## **9. Example Conversion: JSON Array**

**C version:**

```c
json_t *array = json_array();
json_array_append_new(array, json_integer(10));
```

**C++ version:**

```cpp
class JsonArray : public Json {
public:
    void append(int value) {
        json_array_append_new(pImpl->root, json_integer(value));
    }
};
```

**Bonus:** Add iterator support:

```cpp
for (const auto& item : myArray) {
    int value = item.get<int>();
}
```

---

✅ This plan ensures:

* Fully modern C++ API
* Memory safety with RAII
* Strong typing and exceptions
* Clean directory structure
* CMake ready for C++ compilation

---
