find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
link_directories(${OPENSSL_LIB})

set(gmssl_src sm2_ecc.cpp ../../../corecommon/crypto/sm3_hash.cpp sm4_aes.cpp)

include_directories(${PROJECT_SOURCE_DIR}/ypc/stbox/include/ypc/stbox/)
add_library(ypc_gmssl_u SHARED ${gmssl_src})
target_link_libraries(ypc_gmssl_u ${OPENSSL_CRYPTO_LIBRARY} gmssl)
