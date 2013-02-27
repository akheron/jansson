# Notes:
# 
# Author: Paul Harris, June 2012
# 
# Supports: building static/shared, release/debug/etc, can also build html docs and some of the tests.
# Note that its designed for out-of-tree builds, so it will not pollute your source tree.
# 
# TODO 1: Finish implementing tests. api tests are working, but the valgrind variants are not flagging problems.
# 
# TODO 2: There is a check_exports script that would try and incorporate.
# 
# TODO 3: Consolidate version numbers, currently the version number is written into: * cmake (here) * autotools (the configure) * source code header files.  Should not be written directly into header files, autotools/cmake can do that job.
# 
# Brief intro on how to use cmake:
# > mkdir build (somewhere - we do out-of-tree builds)
# > use cmake, ccmake, or cmake-gui to configure the project. for linux, you can only choose one variant: release,debug,etc... and static or shared.
# >> example:
# >> cd build
# >> ccmake -i ../path_to_jansson_dir
# >>  inside, configure your options. press C until there are no lines with * next to them.
# >>  note, I like to configure the 'install' path to ../install, so I get self-contained clean installs I can point other projects to.
# >>  press G to 'generate' the project files.
# >> make (to build the project)
# >> make install
# >> make test (to run the tests, if you enabled them)
# 
# Brief description on how it works:
# There is a small heirachy of CMakeLists.txt files which define how the project is built.
# Header file detection etc is done, and the results are written into config.h and jansson_config.h,
# which are generated from the corresponding config.h.cmake and jansson_config.h.cmake template files.
# The generated header files end up in the build directory - not in the source directory.
# The rest is down to the usual make process.



cmake_minimum_required (VERSION 2.8)
# required for exports? cmake_minimum_required (VERSION 2.8.6)
project (jansson C)

# Options
OPTION (BUILD_SHARED_LIBS "Build shared libraries." OFF)

# Set some nicer output dirs.
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# This is how I thought it should go
# set (JANSSON_VERSION "2.3.1")
# set (JANSSON_SOVERSION 2)

# This is what is required to match the same numbers as automake's
set (JANSSON_VERSION "4.3.1")
set (JANSSON_SOVERSION 4)

# for CheckFunctionKeywords
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

include (CheckFunctionExists)
include (CheckFunctionKeywords)
include (CheckIncludeFiles)
include (CheckTypeSize)

# Turn off Microsofts "security" warnings.
if (MSVC)
   add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo" )
endif()

# Check for the int-type includes
check_include_files (sys/types.h HAVE_SYS_TYPES_H)
check_include_files (inttypes.h HAVE_INTTYPES_H)
check_include_files (stdint.h HAVE_STDINT_H)


# Check our 64 bit integer sizes
check_type_size (__int64 __INT64)
check_type_size (int64_t INT64_T)
check_type_size ("long long" LONG_LONG_INT)

# Check our 32 bit integer sizes
check_type_size (int32_t INT32_T)
check_type_size (__int32 __INT32)
check_type_size ("long" LONG_INT)
check_type_size ("int" INT)

if (HAVE_INT32_T)
   set (JSON_INT32 int32_t)
elseif (HAVE___INT32)
   set (JSON_INT32 __int32)
elseif (HAVE_LONG AND (${LONG_INT} EQUAL 4))
   set (JSON_INT32 long)
elseif (HAVE_INT AND (${INT} EQUAL 4))
   set (JSON_INT32 int)
else ()
   message (FATAL_ERROR "Could not detect a valid 32 bit integer type")
endif ()

# Check for ssize_t and SSIZE_T existance.
check_type_size(ssize_t SSIZE_T)
check_type_size(SSIZE_T UPPERCASE_SSIZE_T)
if(NOT HAVE_SSIZE_T)
   if(HAVE_UPPERCASE_SSIZE_T)
      set(JSON_SSIZE SSIZE_T)
   else()
      set(JSON_SSIZE int)
   endif()
endif()
set(CMAKE_EXTRA_INCLUDE_FILES "")

# Check for all the variants of strtoll
check_function_exists (strtoll HAVE_STRTOLL)
check_function_exists (strtoq HAVE_STRTOQ)
check_function_exists (_strtoi64 HAVE__STRTOI64)

# Figure out what variant we should use
if (HAVE_STRTOLL)
   set (JSON_STRTOINT strtoll)
elseif (HAVE_STRTOQ)
   set (JSON_STRTOINT strtoq)
elseif (HAVE__STRTOI64)
   set (JSON_STRTOINT _strtoi64)
else ()
   # fallback to strtol (32 bit)
   # this will set all the required variables
   set (JSON_STRTOINT strtol)
   set (JSON_INT_T long)
   set (JSON_INTEGER_FORMAT "\"ld\"")
endif ()

# if we haven't defined JSON_INT_T, then we have a 64 bit conversion function.
# detect what to use for the 64 bit type.
# Note: I will prefer long long if I can get it, as that is what the automake system aimed for.
if (NOT DEFINED JSON_INT_T)
   if (HAVE_LONG_LONG_INT AND (${LONG_LONG_INT} EQUAL 8))
      set (JSON_INT_T "long long")
   elseif (HAVE_INT64_T)
      set (JSON_INT_T int64_t)
   elseif (HAVE___INT64)
      set (JSON_INT_T __int64)
   else ()
      message (FATAL_ERROR "Could not detect 64 bit type, although I detected the strtoll equivalent")
   endif ()

   # Apparently, Borland BCC and MSVC wants I64d,
   # Borland BCC could also accept LD
   # and gcc wants ldd,
   # I am not sure what cygwin will want, so I will assume I64d

   if (WIN32) # matches both msvc and cygwin
      set (JSON_INTEGER_FORMAT "\"I64d\"")
   else ()
      set (JSON_INTEGER_FORMAT "\"lld\"")
   endif ()
endif ()


# If locale.h and localeconv() are available, define to 1, otherwise to 0.
check_include_files (locale.h HAVE_LOCALE_H)
check_function_exists (localeconv HAVE_LOCALECONV)

if (HAVE_LOCALECONV AND HAVE_LOCALE_H)
   set (JSON_HAVE_LOCALECONV 1)
else ()
   set (JSON_HAVE_LOCALECONV 0)
endif ()


# check if we have setlocale
check_function_exists (setlocale HAVE_SETLOCALE)


# Check what the inline keyword is.
# Note that the original JSON_INLINE was always set to just 'inline', so this goes further.
check_function_keywords("inline")
check_function_keywords("__inline")
check_function_keywords("__inline__")
# check_function_keywords("__declspec(dllexport)")
# check_function_keywords("__declspec(dllimport)")

if (HAVE_INLINE)
   set (JSON_INLINE inline)
elseif (HAVE___INLINE)
   set (JSON_INLINE __inline)
elseif (HAVE___INLINE__)
   set (JSON_INLINE __inline__)
else (HAVE_INLINE)
   # no inline on this platform
   set (JSON_INLINE)
endif (HAVE_INLINE)

# Find our snprintf
check_function_exists (snprintf HAVE_SNPRINTF)
# check_function_exists (_snprintf_s HAVE__SNPRINTF_s)
check_function_exists (_snprintf HAVE__SNPRINTF)

if (HAVE_SNPRINTF)
   set (JSON_SNPRINTF snprintf)
# elseif (HAVE__SNPRINTF_s)
   # set (JSON_SNPRINTF _snprintf_s)
elseif (HAVE__SNPRINTF)
   set (JSON_SNPRINTF _snprintf)
endif ()

if (MSVC)
   set (CMAKE_DEBUG_POSTFIX "_d")
endif (MSVC)

# configure the public config file
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/src/jansson_config.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/include/jansson_config.h)

# Copy the jansson.h file to the public include folder
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/jansson.h
           DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)


# configure the private config file
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/private_include/config.h)

# and tell the source code to include it
add_definitions (-DHAVE_CONFIG_H)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/include)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/private_include)

# Add the lib sources.
file (GLOB C_FILES src/*.c)

if (BUILD_SHARED_LIBS)

   add_library (jansson SHARED ${C_FILES} jansson.def)

   set_target_properties (jansson PROPERTIES
      VERSION ${JANSSON_VERSION}
      SOVERSION ${JANSSON_SOVERSION})

else ()

   add_library (jansson ${C_FILES})

endif ()

# LIBRARY for linux
# RUNTIME for windows (when building shared)
install (TARGETS jansson
   ARCHIVE DESTINATION lib
   LIBRARY DESTINATION lib
   RUNTIME DESTINATION bin
)


install (FILES ${CMAKE_CURRENT_BINARY_DIR}/include/jansson_config.h DESTINATION include)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/jansson.h DESTINATION include)

# For building Documentation (uses Sphinx)
OPTION (BUILD_DOCS "Build documentation (uses python-sphinx)." OFF)
if (BUILD_DOCS)
   find_package(Sphinx REQUIRED)

   # configured documentation tools and intermediate build results
   set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")

   # Sphinx cache with pickled ReST documents
   set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")

   # HTML output directory
   set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")

   # CMake could be used to build the conf.py file too, 
   # eg it could automatically write the version of the program or change the theme.
   # if(NOT DEFINED SPHINX_THEME)
   #    set(SPHINX_THEME default)
   # endif()
   #
   # if(NOT DEFINED SPHINX_THEME_DIR)
   #    set(SPHINX_THEME_DIR)
   # endif()
   #
   # configure_file(
   #    "${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
   #    "${BINARY_BUILD_DIR}/conf.py"
   #    @ONLY)

   add_custom_target(jansson_docs ALL
      ${SPHINX_EXECUTABLE}
      # -q   # Enable for quiet mode
      -b html
      -d "${SPHINX_CACHE_DIR}"
      # -c "${BINARY_BUILD_DIR}" # enable if using cmake-generated conf.py
      "${CMAKE_CURRENT_SOURCE_DIR}/doc"
      "${SPHINX_HTML_DIR}"
      COMMENT "Building HTML documentation with Sphinx")

   message (STATUS "Documentation has been built in ${SPHINX_HTML_DIR}")
endif ()


OPTION (BUILD_TESTS "Build tests ('make test' to execute tests)" OFF)

if (BUILD_TESTS)
   OPTION (TEST_WITH_VALGRIND "Enable valgrind tests (TODO flag when something is wrong, currently will always pass)" OFF)

   ENABLE_TESTING()

   if (TEST_WITH_VALGRIND)
      # TODO: Add FindValgrind.cmake instead of having a hardcoded path.
      
      # enable valgrind
      set(CMAKE_MEMORYCHECK_COMMAND /usr/bin/valgrind)
      set(CMAKE_MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --show-reachable=yes --track-origins=yes -q")

      set(MEMCHECK_COMMAND "${CMAKE_MEMORYCHECK_COMMAND} ${CMAKE_MEMORYCHECK_COMMAND_OPTIONS}")
      separate_arguments(MEMCHECK_COMMAND)
   endif ()

   #
   # Test suites.
   #
   if (CMAKE_COMPILER_IS_GNUCC)
      add_definitions(-Wall -Wextra -Wdeclaration-after-statement -Werror)
   endif()

   set(api_tests 
         test_array
         test_copy
         test_dump
         test_dump_callback
         test_equal
         test_load
         test_loadb
         test_number
         test_object
         test_pack
         test_simple
         test_unpack)

   # Doing arithmetic on void pointers is not allowed by Microsofts compiler
   # such as secure_malloc and secure_free is doing, so exclude it for now.
   if (NOT MSVC)
      list(APPEND api_tests test_memory_funcs)
   endif()

   # Helper macro for building and linking a test program.
   macro(build_testprog name dir)
       add_executable(${name} EXCLUDE_FROM_ALL ${dir}/${name}.c)
       add_dependencies(${name} jansson)
       target_link_libraries(${name} jansson)
   endmacro(build_testprog)

   # Create executables and tests/valgrind tests for API tests.
   foreach (test ${api_tests})
      build_testprog(${test} ${PROJECT_SOURCE_DIR}/test/suites/api)
      add_test(${test} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${test})

      if (TEST_WITH_VALGRIND)
         add_test(memcheck_${test} ${MEMCHECK_COMMAND} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${test})
      endif()
   endforeach()

   # Test harness for the suites tests.
   build_testprog(json_process ${PROJECT_SOURCE_DIR}/test/bin)

   set(SUITES encoding-flags valid invalid invalid-unicode)
   foreach (SUITE ${SUITES})
       file(GLOB TESTDIRS ${jansson_SOURCE_DIR}/test/suites/${SUITE}/*)
       foreach (TESTDIR ${TESTDIRS})
         if (IS_DIRECTORY ${TESTDIR})
            get_filename_component(TNAME ${TESTDIR} NAME)
            add_test(${SUITE}__${TNAME} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/json_process ${TESTDIR})
         endif()
       endforeach()
   endforeach ()

   add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
                     DEPENDS ${api_tests} json_process)
endif ()

