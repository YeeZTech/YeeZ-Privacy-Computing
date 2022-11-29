
set(T_SRCS ${PROJECT_SOURCE_DIR}/ypc/keymgr/default/gmssl/ekeymgr.cpp
  ${PROJECT_SOURCE_DIR}/ypc/core/src/core_t/ecommon/signer_verify.cpp
  ${PROJECT_SOURCE_DIR}/ypc/core/src/core_t/ecommon/package.cpp
  ${FF_SRC_PATH}/net/common/archive.cpp
  )

set(edl_path ${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox)

add_enclave_library(keymgr_gmssl
  SRCS ${T_SRCS}
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/keymgr/default/enclave/ekeymgr.edl
  EDL_SEARCH_PATHS ${edl_path})

target_include_directories(keymgr_gmssl-edlobj PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_include_directories(keymgr_gmssl PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_libraries(keymgr_gmssl stbox_channel_t stbox_crypto_t gmssl_ecc_t gmssl_t)

enclave_sign(keymgr_gmssl KEY ${PROJECT_SOURCE_DIR}/ypc/keymgr/default/enclave/ekeymgr_private.pem
  CONFIG ${PROJECT_SOURCE_DIR}/ypc/keymgr/default/enclave/ekeymgr.config.xml)

install(FILES ${PROJECT_SOURCE_DIR}/lib/keymgr_gmssl.signed.so DESTINATION lib)
