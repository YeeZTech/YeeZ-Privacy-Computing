add_executable(test_csv main.cpp)
target_link_libraries(test_csv pthread ff_net)
gtest_discover_tests(test_csv)
