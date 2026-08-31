# Plugin-side runtime linked into every Fidelius parser plugin DSO.  It owns
# the enclave lifetime and implements the untrusted half of the parser EDL
# contract on the plugin's behalf, so a plugin only supplies its Edger8r stubs
# and any OCALL its own EDL adds.
add_library(fid_parser_plugin_runtime STATIC
  fid_parser_plugin_runtime.cpp)
set_target_properties(fid_parser_plugin_runtime PROPERTIES
  POSITION_INDEPENDENT_CODE ON)
if(SGX_MODE STREQUAL "Release")
  target_compile_definitions(fid_parser_plugin_runtime PRIVATE
    FID_PARSER_ENCLAVE_DEBUG_FLAG=0)
else()
  target_compile_definitions(fid_parser_plugin_runtime PRIVATE
    FID_PARSER_ENCLAVE_DEBUG_FLAG=SGX_DEBUG_FLAG)
endif()
target_include_directories(fid_parser_plugin_runtime PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${SGX_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_include_directories(fid_parser_plugin_runtime INTERFACE
  "$<INSTALL_INTERFACE:${SGX_INCLUDE_DIR}>")

install(TARGETS fid_parser_plugin_runtime
  EXPORT mod_fid_parser_plugin_runtime
  ARCHIVE DESTINATION "${lib_install_dir}"
  COMPONENT parser_plugin)
install(EXPORT mod_fid_parser_plugin_runtime
  DESTINATION "${config_install_dir}/toolkit"
  NAMESPACE "${namespace}"
  COMPONENT parser_plugin)
