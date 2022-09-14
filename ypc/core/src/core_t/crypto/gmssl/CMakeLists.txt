
set(gmssl_src sm2_ecc.cpp sm3_hash.cpp sm4_aes.cpp)

include_directories(${PROJECT_SOURCE_DIR}/ypc/stbox/include/ypc/stbox/)
add_trusted_library(ypc_gmssl_t
  ypc_gmssl_t
  SRCS "${gmssl_src}"
  EDL ${PROJECT_SOURCE_DIR}/ypc/core/edl/ecc.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/ypc/stbox/src)

target_link_libraries(ypc_gmssl_t stbox_common_t stbox_gmssl_t)
