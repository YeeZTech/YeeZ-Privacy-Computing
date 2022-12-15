configure_file("${CMAKE_CURRENT_SOURCE_DIR}/project.py.in" "${CMAKE_CURRENT_SOURCE_DIR}/project.py")

function(AddPyTest target cmd)
  add_custom_target(${target}
    COMMAND python3 ./test/integrate/${cmd}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
  add_dependencies(post_coverage ${target})
  add_test(NAME ${target}_xx
    COMMAND python3 ./test/integrate/${cmd}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endfunction()

AddPyTest(test_iris test_iris.py)
AddPyTest(test_multi test_findperson_multi.py)
AddPyTest(test_multi_offchain test_findperson_multi_offchain.py)
AddPyTest(test_classifier test_iris_classifier.py)

