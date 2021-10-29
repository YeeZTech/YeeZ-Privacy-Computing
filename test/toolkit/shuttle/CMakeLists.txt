
add_executable(gtest_toolkit_shuttle
  gtest_config.cpp
  main.cpp
  )
target_link_libraries(gtest_toolkit_shuttle gtest ypc_core libypcshuttle)

add_test(NAME test-toolkit-shuttle
  COMMAND gtest_toolkit_shuttle)

add_dependencies(gtest_toolkit_shuttle libypcshuttle)
