add_executable(test_mysql main.cpp)
target_link_libraries(test_mysql pthread ff_net mysqlcppconn)
