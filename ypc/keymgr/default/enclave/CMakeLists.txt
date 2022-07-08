
set(T_SRCS ekeymgr.cpp
  ${PROJECT_SOURCE_DIR}/ypc/core/src/core_t/ecommon/signer_verify.cpp
  ${PROJECT_SOURCE_DIR}/ypc/core/src/core_t/ecommon/package.cpp
  ${FF_SRC_PATH}/net/common/archive.cpp
  )

set(edl_path ${PROJECT_SOURCE_DIR}/ypc/core/edl/:${PROJECT_SOURCE_DIR}/ypc/stbox/src)

add_enclave_library(keymgr
  SRCS ${T_SRCS}
  EDL ekeymgr.edl
  EDL_SEARCH_PATHS "${edl_path}")

target_link_libraries(keymgr stbox_common_t stbox_secp256k1_t stbox_channel_t stbox_crypto_t sgx_ecc_t ypc_stdeth_t ypc_gmssl_t gmssl dl)

enclave_sign(keymgr KEY ekeymgr_private.pem
  CONFIG ekeymgr.config.xml)
install(FILES ${PROJECT_SOURCE_DIR}/lib/keymgr.signed.so DESTINATION lib COMPONENT c_keymgr OPTIONAL)
