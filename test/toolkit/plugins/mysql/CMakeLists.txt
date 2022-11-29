add_executable(test_mysql main.cpp)
target_link_libraries(test_mysql pthread ff_net mysqlcppconn)

#TODO we need fix this
#gtest_discover_tests(test_mysql)
