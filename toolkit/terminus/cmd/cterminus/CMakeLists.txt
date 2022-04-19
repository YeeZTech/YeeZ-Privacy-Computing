add_executable(yterminus main.cpp
  allowance.cpp
  cmd_line.cpp
  crypto.cpp
  forward.cpp
  gen_key.cpp
  helper.cpp
  relay.cpp
  request.cpp)
target_link_libraries(yterminus ypc_terminus glog boost_program_options boost_system ypc_core pthread)
install(TARGETS yterminus DESTINATION bin COMPONENT c_yterminus OPTIONAL)
