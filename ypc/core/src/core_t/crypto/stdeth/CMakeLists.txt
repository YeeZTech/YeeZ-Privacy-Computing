
set(stdeth_src ../../../corecommon/crypto/eth_hash.cpp rijndael128GCM.cpp secp256k1.cpp secp256k1_ecdh_sgx128.cpp)

add_trusted_library(stdeth_t
  stdeth_t
  SRCS "${stdeth_src}"
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/ecc.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/stbox/)

target_include_directories(stdeth_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(stdeth_t PUBLIC stbox_common_t stbox_secp256k1_t stbox_keccak_t)
install(TARGETS stdeth_t EXPORT mod_stdeth_t
  DESTINATION "${lib_install_dir}")

install(EXPORT mod_stdeth_t
  DESTINATION "${config_install_dir}/core"
  NAMESPACE "${namespace}")

AddClangTidy(stdeth_t)
