add_subdirectory(enclave)

include_directories(${PROJECT_SOURCE_DIR}/ypc/stbox/include/ypc/stbox/)
add_executable(gtest_gmssl
  main.cpp
  gtest_gmssl.cpp
  )
target_link_libraries(gtest_gmssl gtest core_gmssl stbox_common core)

set(src main.cpp gtest_crypto.cpp gtest_gmssl.cpp)
set(edl_path ${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox)


add_untrusted_library(test_crypto_lib SHARED
  SRCS "test_crypto_module.cpp"
  EDL ./enclave/enclave.edl
  EDL_SEARCH_PATHS ${edl_path})
target_link_libraries(test_crypto_lib stbox_common core_gmssl)

add_executable(test_crypto main.cpp gtest_crypto.cpp gtest_gmssl.cpp)
add_dependencies(test_crypto test_crypto_enclave-sign)
target_link_libraries(test_crypto test_crypto_lib core gtest)

gtest_discover_tests(test_crypto)
