cmake_minimum_required (VERSION 2.6)
project (jansson)
enable_testing()

set (VERSION 2.2.1)

set (PREFIX "/usr/local" CACHE PATH
     "Install prefix for architecture-independent files" )
set (EXEC_PREFIX ${PREFIX} CACHE PATH
     "Install prefix for architecture-dependent files" )
set (LIBDIR "${EXEC_PREFIX}/lib" CACHE PATH
     "Install location for libraries")

set (BUILD_SHARED OFF CACHE BOOL
     "Build a shared library rather than a static one")

configure_file(jansson.pc.in jansson.pc @ONLY)

set (JANSSON_SRC
    src/dump.c
    src/error.c
    src/hashtable.c
    src/load.c
    src/memory.c
    src/pack_unpack.c
    src/strbuffer.c
    src/strconv.c
    src/utf.c
    src/value.c
    )

if (CMAKE_COMPILER_IS_GNUCC)
    set (CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -Wdeclaration-after-statement -g3 -Werror")
    if (UNIX)
	add_definitions (-fvisibility=hidden)
    endif ()
endif ()

include_directories (. src/)

if (BUILD_SHARED)
    add_library(jansson SHARED ${JANSSON_SRC})
else ()
    add_library(jansson STATIC ${JANSSON_SRC})
endif ()
set_target_properties(jansson PROPERTIES VERSION ${VERSION})

install(TARGETS jansson DESTINATION ${LIBDIR})
install(FILES ${jansson_BINARY_DIR}/jansson.pc DESTINATION "${LIBDIR}/pkgconfig")
install(FILES src/jansson.h DESTINATION "${PREFIX}/include")
install(FILES src/jansson_config.h DESTINATION "${PREFIX}/include")

add_subdirectory (test)

add_custom_target(doc
    COMMAND sphinx-build -b html -d doctrees ${jansson_SOURCE_DIR}/doc html)

# make dist to create a source archive
set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})