configure_file("${CMAKE_CURRENT_SOURCE_DIR}/project.py.in" "${CMAKE_CURRENT_SOURCE_DIR}/project.py")

find_program(LCOV_PATH lcov)

function(AddPyTest target cmd)
  if(LCOV_PATH)
    add_custom_target(${target}
      COMMAND python3 ./test/integrate/${cmd}
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
    add_dependencies(post_coverage ${target})
  endif()
  add_test(NAME ${target}
    COMMAND python3 ./test/integrate/${cmd}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endfunction()

AddPyTest(test_iris test_iris.py)
AddPyTest(test_multi test_findperson_multi.py)

