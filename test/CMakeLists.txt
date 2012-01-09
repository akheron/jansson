add_definitions(-Wall)

macro(build_testprog name dir)
    add_executable(${name} EXCLUDE_FROM_ALL ${dir}/${name}.c)
    add_dependencies (${name} jansson)
    target_link_libraries(${name} jansson)
endmacro(build_testprog)

build_testprog(test_array ./suites/api)
build_testprog(test_copy ./suites/api)
build_testprog(test_dump ./suites/api)
build_testprog(test_dump_callback ./suites/api)
build_testprog(test_load ./suites/api)
build_testprog(test_loadb ./suites/api)
build_testprog(test_memory_funcs ./suites/api)
build_testprog(test_number ./suites/api)
build_testprog(test_object ./suites/api)
build_testprog(test_pack ./suites/api)
build_testprog(test_simple ./suites/api)
build_testprog(test_unpack ./suites/api)

build_testprog(json_process ./bin)


add_test(test_array test_array)
add_test(test_copy test_copy)
add_test(test_dump test_dump)
add_test(test_dump_callback test_dump_callback)
add_test(test_load test_load)
add_test(test_loadb test_loadb)
add_test(test_memory_funcs test_memory_funcs)
add_test(test_number test_number)
add_test(test_object test_object)
add_test(test_pack test_pack)
add_test(test_simple test_simple)
add_test(test_unpack test_unpack)

set(SUITES encoding-flags valid invalid invalid-unicode)
foreach (SUITE ${SUITES})
    file(GLOB TESTDIRS ${jansson_SOURCE_DIR}/test/suites/${SUITE}/*)
    foreach (TESTDIR ${TESTDIRS})
	get_filename_component(TNAME ${TESTDIR} NAME)
	add_test(${SUITE}__${TNAME} json_process ${TESTDIR})
    endforeach()
endforeach ()

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
                  DEPENDS test_array test_copy test_dump test_dump_callback
                  test_load test_loadb test_memory_funcs test_number test_object
                  test_pack test_simple test_unpack json_process)
