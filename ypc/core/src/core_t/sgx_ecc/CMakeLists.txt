add_trusted_library(gmssl_ecc_t
  gmssl_ecc_t
  SRCS gmssl_ecc_impl.cpp
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/ecc.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/stbox/)

add_trusted_library(stdeth_ecc_t
  stdeth_ecc
  SRCS stdeth_ecc_impl.cpp
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/ecc.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/stbox/)
#install(TARGETS sgx_eth_ecc_t DESTINATION lib COMPONENT c_ypc_core OPTIONAL)

target_include_directories(gmssl_ecc_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(gmssl_ecc_t PUBLIC stbox_common_t stbox_crypto_t stbox_secp256k1_t stbox_keccak_t)
install(TARGETS gmssl_ecc_t EXPORT mod_gmssl_ecc_t
  DESTINATION "${lib_install_dir}")

install(EXPORT mod_gmssl_ecc_t
  DESTINATION "${config_install_dir}/core"
  NAMESPACE "${namespace}")

target_include_directories(stdeth_ecc_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(stdeth_ecc_t PUBLIC stbox_common_t stbox_crypto_t stbox_secp256k1_t stbox_keccak_t)
install(TARGETS stdeth_ecc_t EXPORT mod_stdeth_ecc_t
  DESTINATION "${lib_install_dir}")

install(EXPORT mod_stdeth_ecc_t
  DESTINATION "${config_install_dir}/core"
  NAMESPACE "${namespace}")

AddClangTidy(stdeth_ecc_t)
AddClangTidy(gmssl_ecc_t)
