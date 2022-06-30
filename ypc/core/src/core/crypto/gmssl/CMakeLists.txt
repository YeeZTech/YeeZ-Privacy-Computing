
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
link_directories(${OPENSSL_LIB})

set(gmssl_src sm2_ecc.cpp sm3_hash.cpp sm4_aes.cpp)

include_directories(${PROJECT_SOURCE_DIR}/ypc/stbox/include/tsgx)
add_library(ypc_gmssl_u SHARED ${gmssl_src})
#add_trusted_library(ypc_stdeth_t
  #ypc_stdeth_t
  #SRCS "${stdeth_src}"
  #EDL ${PROJECT_SOURCE_DIR}/core/edl/ecc.edl
  #EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/stbox/src)

target_link_libraries(ypc_gmssl_u ${OPENSSL_CRYPTO_LIBRARY} gmssl)
