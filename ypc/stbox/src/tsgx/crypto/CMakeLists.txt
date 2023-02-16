
#add_definitions(-DSTBOX_CRYPTO_VERBOSE)


set(crypto_src
  seal_sgx.cpp
)

add_sgx_library(stbox_crypto_t
  SRCS "${crypto_src}")
target_link_libraries(stbox_crypto_t PUBLIC stbox_common_t)

target_include_directories(stbox_crypto_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ypc/stbox>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ypc/stbox>"
)

install(TARGETS stbox_crypto_t EXPORT mod_crypto_t
  DESTINATION "${lib_install_dir}")

install(EXPORT mod_crypto_t
  DESTINATION "${config_install_dir}/stbox"
  NAMESPACE "${namespace}")

AddClangTidy(stbox_crypto_t)
