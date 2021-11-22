add_executable(yterminus main.cpp)

target_link_libraries(yterminus ypc_terminus glog boost_program_options boost_system ypc_core)
