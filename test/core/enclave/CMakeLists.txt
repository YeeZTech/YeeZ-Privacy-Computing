
set(T_SRCS enclave.cpp
  )

set(edl_path ${PROJECT_SOURCE_DIR}/stbox/src/tsgx/crypto/:${PROJECT_SOURCE_DIR}/stbox/src)

add_enclave_library(test_ypc_enclave
  SRCS ${T_SRCS}
  EDL enclave.edl
  EDL_SEARCH_PATHS "${edl_path}")

target_link_libraries(test_ypc_enclave stbox_common_t stbox_secp256k1_t stbox_channel_t stbox_crypto_t)

enclave_sign(test_ypc_enclave KEY enclave_private.pem
  CONFIG enclave.config.xml)
