
add_executable(gtest_keymgr
  main.cpp)
target_link_libraries(gtest_keymgr gtest pthread)
gtest_discover_tests(gtest_keymgr)
AddCoverage(gtest_keymgr)
