add_subdirectory(enclave)


set(edl_path ${PROJECT_SOURCE_DIR}/stbox/src/tsgx/crypto/:${PROJECT_SOURCE_DIR}/stbox/src)

add_untrusted_library(keymgr_module SHARED
  SRCS "keymgr_sgx_module.cpp"
  EDL ./enclave/ekeymgr.edl
  EDL_SEARCH_PATHS ${edl_path})
target_link_libraries(keymgr_module stbox_common_u ypc_core keymgr_utils)

add_sgx_executable(keymgr_tool
  SRCS "main.cpp")
add_dependencies(keymgr_tool keymgr-sign)
target_link_libraries(keymgr_tool keymgr_module)

