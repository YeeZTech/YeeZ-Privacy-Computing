
set(stdeth_src ../../../corecommon/crypto/eth_hash.cpp rijndael128GCM.cpp secp256k1.cpp secp256k1_ecdh_sgx128.cpp)

add_trusted_library(ypc_stdeth_t
  ypc_stdeth_t
  SRCS "${stdeth_src}"
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/ecc.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/stbox/)

target_link_libraries(ypc_stdeth_t stbox_common_t stbox_secp256k1_t stbox_keccak_t)
install(TARGETS ypc_stdeth_t DESTINATION lib COMPONENT c_ypc_core OPTIONAL)
