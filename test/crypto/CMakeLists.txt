add_subdirectory(enclave)

set(src main.cpp gtest_crypto.cpp)
set(edl_path ${PROJECT_SOURCE_DIR}/core/edl/:${PROJECT_SOURCE_DIR}/stbox/src)


add_untrusted_library(test_crypto_lib SHARED
  SRCS "test_crypto_module.cpp"
  EDL ./enclave/enclave.edl
  EDL_SEARCH_PATHS ${edl_path})

target_link_libraries(test_crypto_lib stbox_common_u)

add_executable(test_crypto main.cpp gtest_crypto.cpp)

add_dependencies(test_crypto test_crypto_enclave-sign)
target_link_libraries(test_crypto test_crypto_lib ypc_core gtest)

add_test(NAME test_crypto
  COMMAND test_crypto)
