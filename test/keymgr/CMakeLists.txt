
add_executable(gtest_keymgr
  main.cpp)
target_link_libraries(gtest_keymgr gtest )

add_test(NAME test-keymgr
  COMMAND gtest_keymgr)
