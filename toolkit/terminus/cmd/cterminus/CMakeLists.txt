add_executable(yterminus main.cpp
  allowance.cpp
  cmd_line.cpp
  crypto.cpp
  forward.cpp
  gen_key.cpp
  helper.cpp
  relay.cpp
  request.cpp
  sign.cpp)
target_link_libraries(yterminus terminus glog boost_program_options boost_system core)

target_include_directories(yterminus PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )
install(TARGETS yterminus DESTINATION bin)
AddClangTidy(yterminus)
