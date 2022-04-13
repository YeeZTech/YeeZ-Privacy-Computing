add_executable(yterminus main.cpp)
target_link_libraries(yterminus ypc_terminus glog boost_program_options boost_system ypc_core pthread)
install(TARGETS yterminus DESTINATION bin COMPONENT c_yterminus OPTIONAL)
