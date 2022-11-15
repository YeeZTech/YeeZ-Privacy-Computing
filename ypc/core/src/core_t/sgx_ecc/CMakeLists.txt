add_trusted_library(sgx_gmssl_ecc_t
  sgx_gmssl_ecc_t
  SRCS gmssl_ecc_impl.cpp
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/ecc.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/stbox/)
install(TARGETS sgx_gmssl_ecc_t DESTINATION lib COMPONENT c_ypc_core OPTIONAL)

add_trusted_library(sgx_eth_ecc_t
  sgx_eth_ecc_t
  SRCS stdeth_ecc_impl.cpp
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/ecc.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/stbox/)
install(TARGETS sgx_eth_ecc_t DESTINATION lib COMPONENT c_ypc_core OPTIONAL)
