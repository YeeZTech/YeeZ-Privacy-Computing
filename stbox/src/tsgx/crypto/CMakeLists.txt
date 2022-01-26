
#add_definitions(-DSTBOX_CRYPTO_VERBOSE)


set(crypto_src
  ecp.cpp
  seal_sgx.cpp
  secp256k1/ecc_secp256k1.cpp
  secp256k1/secp256k1_context_i.cpp
  )

add_sgx_library(stbox_crypto_t stbox_crypto_t
  SRCS "${crypto_src}")
install(TARGETS stbox_crypto_t DESTINATION lib)

target_link_libraries(stbox_crypto_t stbox_common_t stbox_secp256k1_t)
