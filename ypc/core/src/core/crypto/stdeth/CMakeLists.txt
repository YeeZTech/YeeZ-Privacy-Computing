find_package(OpenSSL REQUIRED)

set(stdeth_src ../../../corecommon/crypto/eth_hash.cpp rijndael128GCM.cpp secp256k1.cpp secp256k1_ecdh_sgx128.cpp openssl.cpp)

add_library(core_stdeth SHARED ${stdeth_src})
#target_link_libraries(ypc_stdeth_u ${OPENSSL_CRYPTO_LIBRARY} stbox_keccak_u secp256k1)
#install(TARGETS ypc_stdeth_u DESTINATION lib COMPONENT c_ypc_core OPTIONAL)

target_include_directories(core_stdeth PUBLIC
  "${OPENSSL_INCLUDE_DIR}"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")


target_link_libraries(core_stdeth PUBLIC ${OPENSSL_CRYPTO_LIBRARY} stbox_keccak stbox_secp256k1)

install(TARGETS core_stdeth EXPORT mod_core_stdeth
  DESTINATION "${lib_install_dir}"
  COMPONENT stdeth)

install(EXPORT mod_core_stdeth
  DESTINATION "${config_install_dir}/core"
  NAMESPACE "${namespace}"
  COMPONENT stdeth
  )

AddClangTidy(core_stdeth)
EnableCoverage(core_stdeth)
add_version(core_stdeth)
