set (tests 
   test_array
   test_copy
   test_dump
   test_dump_callback
   test_equal
   test_load
   test_loadb
   test_memory_funcs
   test_number
   test_object
   test_pack
   test_simple
   test_unpack)

foreach (test ${tests})
   add_executable (${test} ${test}.c)
   target_link_libraries (${test} jansson)
endforeach()

# batch plain tests
foreach (test ${tests})
   add_test (${test} ${CMAKE_CURRENT_BINARY_DIR}/${test})
endforeach()

# batch valgrind tests
if (TEST_WITH_VALGRIND)
   foreach (test ${tests})
      add_test (memcheck_${test} ${MEMCHECK_COMMAND} ${CMAKE_CURRENT_BINARY_DIR}/${test})
   endforeach()
endif ()
