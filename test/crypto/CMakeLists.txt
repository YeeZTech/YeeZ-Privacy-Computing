add_subdirectory(enclave)

include_directories(${PROJECT_SOURCE_DIR}/stbox/include/stbox/tsgx)
add_executable(gtest_gmssl
  main.cpp
  gtest_gmssl.cpp
  )
target_link_libraries(gtest_gmssl gtest ypc_gmssl_u stbox_common_u ypc_core)

set(src main.cpp gtest_crypto.cpp gtest_gmssl.cpp)
set(edl_path ${PROJECT_SOURCE_DIR}/core/edl/:${PROJECT_SOURCE_DIR}/stbox/src)


add_untrusted_library(test_crypto_lib SHARED
  SRCS "test_crypto_module.cpp"
  EDL ./enclave/enclave.edl
  EDL_SEARCH_PATHS ${edl_path})

target_link_libraries(test_crypto_lib stbox_common_u ypc_gmssl_u)

add_executable(test_crypto main.cpp gtest_crypto.cpp gtest_gmssl.cpp)

add_dependencies(test_crypto test_crypto_enclave-sign)
target_link_libraries(test_crypto test_crypto_lib ypc_core gtest)

add_test(NAME test_crypto
  COMMAND test_crypto)

