set(gmssl_src
	sha256.c
	sm3.c
	asn1.c
	sm2_alg.c
	sm2_key.c
	sm2_lib.c
	sm4_common.c
	sm4_setkey.c
	sm4_enc.c
	aes.c
	aes_modes.c
	hex.c
	gf128.c
	gcm.c
	sm4_modes.c
	block_cipher.c
  )

add_library(stbox_gmssl
  SHARED
  ${gmssl_src}
  )
target_include_directories(stbox_gmssl PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ypc/stbox>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ypc/stbox>"
)

install(TARGETS stbox_gmssl EXPORT mod_gmssl
  DESTINATION lib
)
install(EXPORT mod_gmssl
  DESTINATION "${config_install_dir}/stbox"
  NAMESPACE "${namespace}")

add_version(stbox_gmssl)

if(SGX_FOUND)
add_sgx_library(stbox_gmssl_t
  stbox_gmssl_t
  SRCS "${gmssl_src}")

target_include_directories(stbox_gmssl_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ypc/stbox>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ypc/stbox>"
)

install(TARGETS stbox_gmssl_t EXPORT mod_gmssl_t
  DESTINATION lib COMPONENT c_stbox_common OPTIONAL)

install(EXPORT mod_gmssl_t
  DESTINATION "${config_install_dir}/stbox"
  NAMESPACE "${namespace}")

endif()

