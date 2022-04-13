add_trusted_library(sgx_ecc_t
  sgx_ecc
  SRCS ecc_impl.cpp
  EDL ../../../edl/ecc.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/stbox/src/)
target_link_libraries(sgx_ecc_t stbox_crypto_t)
#install(TARGETS sgx_ecc_t DESTINATION lib COMPONENT c_ypc_core OPTIONAL)
