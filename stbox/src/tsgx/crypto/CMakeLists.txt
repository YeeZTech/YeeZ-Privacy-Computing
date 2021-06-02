
#add_definitions(-DSTBOX_CRYPTO_VERBOSE)

add_trusted_library(stbox_crypto_t
  stbox_crypto
  SRCS ecc.cpp ecp.cpp ecc_sgx_impl.cpp ecc_context.cpp
  EDL ./ecc.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/stbox/src/)
install(TARGETS stbox_crypto_t DESTINATION lib)

target_link_libraries(stbox_crypto_t stbox_common_t stbox_secp256k1_t)
