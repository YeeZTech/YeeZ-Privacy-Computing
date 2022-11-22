add_subdirectory(enclave)
add_subdirectory(gmssl)

set(edl_path ${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox)

add_untrusted_library(keymgr_module SHARED
  SRCS "keymgr_sgx_module.cpp"
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/keymgr/default/enclave/ekeymgr.edl
  EDL_SEARCH_PATHS ${edl_path})
target_link_libraries(keymgr_module stbox_common_u keymgr_utils)
install(TARGETS keymgr_module DESTINATION lib COMPONENT c_keymgr OPTIONAL)

add_sgx_executable(keymgr_tool
  SRCS main.cpp no_use_extra_data_bridge.cpp)
add_dependencies(keymgr_tool keymgr-sign)
target_link_libraries(keymgr_tool keymgr_module ypc_common_u)
install(TARGETS keymgr_tool DESTINATION bin COMPONENT c_keymgr OPTIONAL)

