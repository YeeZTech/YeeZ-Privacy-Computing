add_subdirectory(enclave)
add_subdirectory(gmssl)

set(edl_path ${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox)

add_untrusted_library(keymgr_module SHARED
  SRCS "keymgr_sgx_module.cpp"
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/keymgr/default/enclave/ekeymgr.edl
  EDL_SEARCH_PATHS ${edl_path})
target_include_directories(keymgr_module INTERFACE
  ${SGX_INCLUDE_DIR}
  )
target_link_libraries(keymgr_module stbox_common keymgr_utils)
install(TARGETS keymgr_module DESTINATION lib)

add_sgx_executable(keymgr_tool
  SRCS main.cpp no_use_extra_data_bridge.cpp)
add_dependencies(keymgr_tool keymgr-sign)
target_link_libraries(keymgr_tool keymgr_module common)
install(TARGETS keymgr_tool DESTINATION bin )

