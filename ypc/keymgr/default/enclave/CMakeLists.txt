
set(T_SRCS ekeymgr.cpp
  ${PROJECT_SOURCE_DIR}/ypc/core/src/core_t/ecommon/signer_verify.cpp
  ${PROJECT_SOURCE_DIR}/ypc/core/src/core_t/ecommon/package.cpp
  ${FF_SRC_PATH}/net/common/archive.cpp
  )

set(edl_path ${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox)

add_enclave_library(keymgr
  SRCS ${T_SRCS}
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/keymgr/default/enclave/ekeymgr.edl
  EDL_SEARCH_PATHS ${edl_path})

target_link_libraries(keymgr stbox_common_t stbox_channel_t stbox_crypto_t sgx_eth_ecc_t ypc_stdeth_t dl)

if(SGX_MODE STREQUAL "Debug")
  enclave_sign(keymgr KEY ekeymgr_private.pem
    CONFIG ekeymgr.config.debug.xml)
else()
  enclave_sign(keymgr KEY ekeymgr_private.pem
    CONFIG ekeymgr.config.xml)
endif()
install(FILES ${PROJECT_SOURCE_DIR}/lib/keymgr.signed.so DESTINATION lib COMPONENT c_keymgr OPTIONAL)
