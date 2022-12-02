#include_directories(${PROJECT_SOURCE_DIR}/toolkit/analyzer/)
add_executable(fid_analyzer main.cpp
  sgx_bridge.cpp
  parsers/parser.cpp
  )

target_link_libraries(fid_analyzer
  stbox_common
  core
  core_parser_module
  keymgr_module
  keymgr_utils
  glog
  )
target_include_directories(fid_analyzer PRIVATE
  "${PROJECT_SOURCE_DIR}/toolkit/analyzer/"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  )

install(TARGETS fid_analyzer DESTINATION bin)
AddClangTidy(fid_analyzer)
