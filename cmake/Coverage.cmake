
find_program(LCOV_PATH lcov)
find_program(GENHTML_PATH genhtml)
if(LCOV_PATH AND GENHTML_PATH)
  add_custom_target(pre_coverage COMMENT "Prepare coverage"
    COMMAND ${LCOV_PATH} -d . --zerocounters
    )
  add_custom_target(post_coverage COMMENT "Running coverage"
    COMMAND ${LCOV_PATH} -d . --capture -o coverage.info
    COMMAND ${LCOV_PATH} -r coverage.info '/usr/include/*'
                         -o filtered.info
    COMMAND ${GENHTML_PATH} -o coverage filtered.info
      --legend
    COMMAND rm -rf coverage.info filtered.info
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
  add_custom_target(coverage)
  add_dependencies(coverage post_coverage)
endif()



function(AddCoverage target)
  find_program(LCOV_PATH lcov)
  find_program(GENHTML_PATH genhtml)
  if(NOT LCOV_PATH)
    message(WARNING "not found lcov, ignore coverage")
    return()
  endif()
  if(NOT GENHTML_PATH)
    message(WARNING "not found genhtml, ignore coverage")
    return()
  endif()

  target_compile_options(${target} PRIVATE --coverage)
  target_link_options(${target} PUBLIC --coverage)

  add_custom_command(TARGET ${target} PRE_BUILD
    COMMAND find ${CMAKE_BINARY_DIR} -type f -name '*.gcda' -exec rm {} +)
  add_custom_target(${target}-coverage
    COMMAND $<TARGET_FILE:${target}>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  add_dependencies(${target}-coverage pre_coverage)
  add_dependencies(post_coverage ${target}-coverage)
endfunction()

function(EnableCoverage target)
  if(CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_options(${target} PRIVATE --coverage -fprofile-arcs -ftest-coverage)
    target_link_options(${target} PUBLIC --coverage)

    add_custom_command(TARGET ${target} PRE_BUILD
      COMMAND find ${CMAKE_BINARY_DIR} -type f -name '*.gcda' -exec rm {} +)
  endif()
endfunction()
