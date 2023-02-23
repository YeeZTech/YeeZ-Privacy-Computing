find_package(OpenSSL REQUIRED)
#include_directories(${OPENSSL_INCLUDE_DIR})
#link_directories(${OPENSSL_LIB})

set(gmssl_src sm2_ecc.cpp ../../../corecommon/crypto/sm3_hash.cpp sm4_aes.cpp)

add_library(core_gmssl SHARED ${gmssl_src})
target_include_directories(core_gmssl PUBLIC
  "${OPENSSL_INCLUDE_DIR}"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_directories(core_gmssl PUBLIC ${OPENSSL_LIB})

target_link_libraries(core_gmssl PUBLIC ${OPENSSL_CRYPTO_LIBRARY} stbox_gmssl)

install(TARGETS core_gmssl EXPORT mod_core_gmssl
  DESTINATION "${lib_install_dir}"
  COMPONENT gmssl)

install(EXPORT mod_core_gmssl
  DESTINATION "${config_install_dir}/core"
  NAMESPACE "${namespace}"
  COMPONENT gmssl
  )

AddClangTidy(core_gmssl)
EnableCoverage(core_gmssl)
add_version(core_gmssl)
