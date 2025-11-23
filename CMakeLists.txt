cmake_minimum_required (VERSION 3.10)
project(jansson C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(JANSSON_BUILD_SHARED_LIBS "Build shared libraries." OFF)
option(JANSSON_BUILD_DOCS "Build documentation." OFF)
option(JANSSON_EXAMPLES "Build examples." OFF)
option(JANSSON_WITHOUT_TESTS "Build without tests." ON)
option(JANSSON_INSTALL "Generate install target." ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Debug)
endif()

# Include modules
include(CheckCXXCompilerFlag)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckTypeSize)
include(TestBigEndian)

# Check for compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Check for headers
set(CMAKE_EXTRA_INCLUDE_FILES "")
check_include_files("endian.h" HAVE_ENDIAN_H)
check_include_files("fcntl.h" HAVE_FCNTL_H)
check_include_files("sched.h" HAVE_SCHED_H)
check_include_files("unistd.h" HAVE_UNISTD_H)
check_include_files("sys/param.h" HAVE_SYS_PARAM_H)
check_include_files("sys/stat.h" HAVE_SYS_STAT_H)
check_include_files("sys/time.h" HAVE_SYS_TIME_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)

# Check for functions
check_function_exists("close" HAVE_CLOSE)
check_function_exists("getpid" HAVE_GETPID)
check_function_exists("gettimeofday" HAVE_GETTIMEOFDAY)
check_function_exists("open" HAVE_OPEN)
check_function_exists("read" HAVE_READ)
check_function_exists("sched_yield" HAVE_SCHED_YIELD)

# Check for types
check_type_size("__int64" __INT64)
check_type_size("int64_t" INT64_T)
check_type_size("long long" LONG_LONG)
check_type_size("int32_t" INT32_T)
check_type_size("__int32" __INT32)
check_type_size("long" LONG)
check_type_size("int" INT)
check_type_size("unsigned long" UNSIGNED_LONG)
check_type_size("unsigned int" UNSIGNED_INT)
check_type_size("unsigned short" UNSIGNED_SHORT)
check_type_size("uint32_t" UINT32_T)
check_type_size("__uint32" __UINT32)
check_type_size("uint16_t" UINT16_T)
check_type_size("__uint16" __UINT16)
check_type_size("uint8_t" UINT8_T)
check_type_size("__uint8" __UINT8)
check_type_size("ssize_t" SSIZE_T)
check_type_size("SSIZE_T" _SSIZE_T)

# Check for strtoll and strtoq
check_function_exists("strtoll" HAVE_STRTOLL)
check_function_exists("strtoq" HAVE_STRTOQ)
check_function_exists("_strtoi64" HAVE__STRTOI64)

# Check for locale
check_include_files("locale.h" HAVE_LOCALE_H)
check_function_exists("setlocale" HAVE_SETLOCALE)

# Check for inline
set(CMAKE_CXX_FLAGS_SAVE "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DINLINE=inline")
check_cxx_source_compiles("inline static void f(void) {} int main(void) { return 0; }" HAVE_INLINE)
if(NOT HAVE_INLINE)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVE} -DINLINE=__inline")
   check_cxx_source_compiles("__inline static void f(void) {} int main(void) { return 0; }" HAVE___INLINE)
   if(NOT HAVE___INLINE)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVE} -DINLINE=__inline__")
      check_cxx_source_compiles("__inline__ static void f(void) {} int main(void) { return 0; }" HAVE___INLINE__)
      if(NOT HAVE___INLINE__)
         set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVE} -DINLINE=")
      endif()
   endif()
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVE}")

# Check for __sync builtins
check_cxx_source_compiles("
   int main(void) {
      volatile int n = 0;
      __sync_add_and_fetch(&n, 1);
      return 0;
   }" HAVE_SYNC_BUILTINS)

# Check for __atomic builtins
check_cxx_source_compiles("
   int main(void) {
      volatile int n = 0;
      __atomic_add_fetch(&n, 1, __ATOMIC_SEQ_CST);
      return 0;
   }" HAVE_ATOMIC_BUILTINS)

# Source files
set(JANSSON_SOURCES
   jansson-cpp/src/dump.cpp
   jansson-cpp/src/error.cpp
   jansson-cpp/src/hashtable.cpp
   jansson-cpp/src/hashtable_seed.cpp
   jansson-cpp/src/load.cpp
   jansson-cpp/src/memory.cpp
   jansson-cpp/src/pack_unpack.cpp
   jansson-cpp/src/strbuffer.cpp
   jansson-cpp/src/strconv.cpp
   jansson-cpp/src/utf.cpp
   jansson-cpp/src/value.cpp
   jansson-cpp/src/version.cpp
)

# Header files
set(JANSSON_HEADERS
   jansson-cpp/include/jansson.hpp
   jansson-cpp/include/jansson_config.hpp
)

# Private header files
set(JANSSON_PRIVATE_HEADERS
   jansson-cpp/include/hashtable.hpp
   jansson-cpp/include/jansson_private.hpp
   jansson-cpp/include/lookup3.hpp
   jansson-cpp/include/strbuffer.hpp
   jansson-cpp/include/utf.hpp
)

# Create library
if(JANSSON_BUILD_SHARED_LIBS)
   add_library(jansson SHARED ${JANSSON_SOURCES} ${JANSSON_HEADERS} ${JANSSON_PRIVATE_HEADERS})
else()
   add_library(jansson STATIC ${JANSSON_SOURCES} ${JANSSON_HEADERS} ${JANSSON_PRIVATE_HEADERS})
endif()

# Set properties
target_include_directories(jansson PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/jansson-cpp/include>
   $<INSTALL_INTERFACE:include>
)

# Set library properties
set_target_properties(jansson PROPERTIES
   VERSION 2.14
   SOVERSION 2
   PUBLIC_HEADER "${JANSSON_HEADERS}"
)

# Install rules
if(JANSSON_INSTALL)
   install(TARGETS jansson
      EXPORT jansson-targets
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
      RUNTIME DESTINATION bin
      PUBLIC_HEADER DESTINATION include
      INCLUDES DESTINATION include
   )

   install(EXPORT jansson-targets
      FILE jansson-targets.cmake
      NAMESPACE jansson::
      DESTINATION lib/cmake/jansson
   )

   # Generate and install CMake config files
   include(CMakePackageConfigHelpers)
   
   configure_package_config_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake/jansson-config.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/jansson-config.cmake"
      INSTALL_DESTINATION lib/cmake/jansson
   )

   write_basic_package_version_file(
      "${CMAKE_CURRENT_BINARY_DIR}/jansson-config-version.cmake"
      VERSION 2.14
      COMPATIBILITY SameMajorVersion
   )

   install(FILES
      "${CMAKE_CURRENT_BINARY_DIR}/jansson-config.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/jansson-config-version.cmake"
      DESTINATION lib/cmake/jansson
   )
endif()

# Examples
if(JANSSON_EXAMPLES)
   add_subdirectory(examples)
endif()

# Tests
if(NOT JANSSON_WITHOUT_TESTS)
   enable_testing()
   add_subdirectory(test)
endif()

# Documentation
if(JANSSON_BUILD_DOCS)
   find_package(Doxygen)
   if(DOXYGEN_FOUND)
      add_subdirectory(doc)
   else()
      message(WARNING "Doxygen not found, documentation will not be built")
   endif()
endif()
