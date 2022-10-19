
set(gmssl_src sm2_ecc.cpp ../../../corecommon/crypto/sm3_hash.cpp sm4_aes.cpp)

add_trusted_library(ypc_gmssl_t
  ypc_gmssl_t
  SRCS "${gmssl_src}"
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/ecc.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/stbox/)

target_link_libraries(ypc_gmssl_t stbox_common_t stbox_gmssl_t)
install(TARGETS ypc_gmssl_t DESTINATION lib COMPONENT c_ypc_core OPTIONAL)
