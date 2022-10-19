
include_directories(${PROJECT_SOURCE_DIR}/toolkit/analyzer/)
add_executable(fid_analyzer main.cpp
  sgx_bridge.cpp
  parsers/parser.cpp
  )

target_link_libraries(fid_analyzer
  stbox_common_u
  ypc_core
  ypc_parser_module
  keymgr_module
  keymgr_utils
  glog
  ${ypc_dep_libs})

install(TARGETS fid_analyzer DESTINATION bin COMPONENT c_analyzer OPTIONAL)
