
include_directories(${PROJECT_SOURCE_DIR}/ypc/stbox/include/ypc/stbox/)
add_executable(gtest_gmssl
  main.cpp
  gtest_gmssl.cpp
  )
target_link_libraries(gtest_gmssl gtest core_gmssl stbox_common core)
gtest_discover_tests(gtest_gmssl)
AddCoverage(gtest_gmssl)

if(SGX_FOUND)
add_subdirectory(enclave)

set(src main.cpp gtest_crypto.cpp gtest_gmssl.cpp)
set(edl_path ${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox)


add_untrusted_library(test_crypto_lib SHARED
  SRCS "test_crypto_module.cpp"
  EDL ./enclave/enclave.edl
  EDL_SEARCH_PATHS ${edl_path})
target_link_libraries(test_crypto_lib PUBLIC stbox_common core_gmssl)

add_executable(test_crypto main.cpp gtest_crypto.cpp gtest_gmssl.cpp)
add_dependencies(test_crypto test_crypto_enclave-sign)
target_link_libraries(test_crypto test_crypto_lib core gtest)

target_include_directories(test_crypto PRIVATE "${SGX_INCLUDE_DIR}")
target_link_directories(test_crypto PRIVATE "${SGX_LIBRARY_DIR}")

gtest_discover_tests(test_crypto)
AddCoverage(test_crypto)

endif()

add_executable(gtest_group main.cpp gtest_group.cpp)
target_link_libraries(gtest_group gtest core common core_stdeth core_gmssl terminus)
gtest_discover_tests(gtest_group)
AddCoverage(gtest_group)
