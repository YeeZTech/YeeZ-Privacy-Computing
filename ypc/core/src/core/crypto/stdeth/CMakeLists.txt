
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
link_directories(${OPENSSL_LIB})

set(stdeth_src eth_hash.cpp rijndael128GCM.cpp secp256k1.cpp secp256k1_ecdh_sgx128.cpp openssl.cpp)

add_library(ypc_stdeth_u SHARED ${stdeth_src})
target_link_libraries(ypc_stdeth_u ${OPENSSL_CRYPTO_LIBRARY} stbox_keccak_u secp256k1)
