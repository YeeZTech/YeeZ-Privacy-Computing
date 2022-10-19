find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
link_directories(${OPENSSL_LIB})

set(gmssl_src sm2_ecc.cpp ../../../corecommon/crypto/sm3_hash.cpp sm4_aes.cpp)

add_library(ypc_gmssl_u SHARED ${gmssl_src})
target_link_libraries(ypc_gmssl_u ${OPENSSL_CRYPTO_LIBRARY} gmssl)
install(TARGETS ypc_gmssl_u DESTINATION lib COMPONENT c_ypc_core OPTIONAL)
