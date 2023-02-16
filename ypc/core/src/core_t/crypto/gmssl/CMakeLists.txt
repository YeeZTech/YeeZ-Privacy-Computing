
set(gmssl_src sm2_ecc.cpp ../../../corecommon/crypto/sm3_hash.cpp sm4_aes.cpp)

add_trusted_library(gmssl_t
  gmssl_t
  SRCS "${gmssl_src}"
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/ecc.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/stbox/)

target_include_directories(gmssl_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(gmssl_t PUBLIC stbox_common_t stbox_gmssl_t)
install(TARGETS gmssl_t EXPORT mod_gmssl_t
  DESTINATION "${lib_install_dir}")

install(EXPORT mod_gmssl_t
  DESTINATION "${config_install_dir}/core"
  NAMESPACE "${namespace}")

AddClangTidy(gmssl_t)
