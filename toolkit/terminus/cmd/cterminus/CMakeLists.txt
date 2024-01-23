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

add_library(yterminus_lib lib.cpp
  allowance.cpp
  cmd_line.cpp
  crypto.cpp
  forward.cpp
  gen_key.cpp
  helper.cpp
  relay.cpp
  request.cpp
  sign.cpp)
set_property(TARGET yterminus_lib PROPERTY OUTPUT_NAME yterminus)
#set_target_properties(yterminus_lib PROPERTIES
#        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
target_link_libraries(yterminus_lib terminus glog boost_program_options boost_system core)
install(TARGETS yterminus_lib
  DESTINATION "${lib_install_dir}") 

install(TARGETS yterminus
  DESTINATION "${bin_install_dir}")
AddClangTidy(yterminus)
EnableCoverage(yterminus)
