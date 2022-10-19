
include_directories(${PROJECT_SOURCE_DIR}/toolkit/analyzer/)
add_executable(fid_analyzer main.cpp
  sgx_bridge.cpp
  #parsers/file_parser.cpp
  parsers/parser.cpp
  #params/param_from_json.cpp
  #params/param_from_memory.cpp
  #params/param_from_db.cpp
  #result/result_to_json.cpp
  #result/result_to_db.cpp
  #db/db.cpp
  #check_data.cpp
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
