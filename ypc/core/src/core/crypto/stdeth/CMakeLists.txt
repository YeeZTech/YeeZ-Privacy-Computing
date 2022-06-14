
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
link_directories(${OPENSSL_LIB})

set(stdeth_src eth_hash.cpp rijndael128GCM.cpp secp256k1.cpp secp256k1_ecdh_sgx128.cpp openssl.cpp)

add_library(ypc_stdeth_u SHARED ${stdeth_src})
#add_trusted_library(ypc_stdeth_t
  #ypc_stdeth_t
  #SRCS "${stdeth_src}"
  #EDL ${PROJECT_SOURCE_DIR}/core/edl/ecc.edl
  #EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/stbox/src)

target_link_libraries(ypc_stdeth_u ${OPENSSL_CRYPTO_LIBRARY} stbox_keccak_u secp256k1)
