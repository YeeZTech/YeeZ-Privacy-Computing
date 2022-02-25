add_executable(yterminus EXCLUDE_FROM_ALL  main.cpp)
install(TARGETS yterminus DESTINATION bin COMPONENT yterminus-runtime OPTIONAL)
target_link_libraries(yterminus ypc_terminus glog boost_program_options boost_system ypc_core)
