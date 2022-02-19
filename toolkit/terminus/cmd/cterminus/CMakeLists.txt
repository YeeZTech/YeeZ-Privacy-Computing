add_executable(yterminus main.cpp
  allowance.cpp
  cmd_line.cpp
  crypto.cpp
  forward.cpp
  gen_key.cpp
  helper.cpp
  request.cpp)

target_link_libraries(yterminus ypc_terminus glog boost_program_options boost_system ypc_core)
