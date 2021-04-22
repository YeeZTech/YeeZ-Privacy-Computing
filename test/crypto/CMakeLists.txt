add_subdirectory(enclave)

set(edl_path ${PROJECT_SOURCE_DIR}/stbox/src/tsgx/crypto/:${PROJECT_SOURCE_DIR}/stbox/src)
add_untrusted_executable(test_crypto
  SRCS main.cpp
  EDL ./enclave/enclave.edl
  EDL_SEARCH_PATHS ${edl_path})

add_dependencies(test_crypto test_crypto_enclave-sign)
target_link_libraries(test_crypto stbox_common_u ypc_core )
