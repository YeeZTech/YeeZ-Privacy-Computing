find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
link_directories(${OPENSSL_LIB})
set(src crypto_pack.cpp
  sm_compatible.cpp
  sgx_and_eth_compatible/crypto_pack.cpp
  sgx_and_eth_compatible/openssl.cpp
  single_data_onchain_result.cpp
  ${PROJECT_SOURCE_DIR}/vendor/keccak/keccak.c
  )

add_library(ypc_terminus SHARED ${src})
target_link_libraries(ypc_terminus ${OPENSSL_CRYPTO_LIBRARY} secp256k1)
install(TARGETS ypc_terminus DESTINATION lib COMPONENT c_yterminus OPTIONAL)
