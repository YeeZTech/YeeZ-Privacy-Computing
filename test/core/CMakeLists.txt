
set(srcs
  main.cpp
  gtest_common.cpp
  gtest_byte.cpp
  gtest_base58.cpp
  gtest_base64.cpp
  gtest_command_executor.cpp
  gtest_filesystem.cpp
  gtest_configuration.cpp
  gtest_blockfile.cpp
  gtest_sealed_file.cpp
  gtest_ntobject_file.cpp
  gtest_ntjson.cpp
  )

if(SGX_FOUND)
  set(srcs ${srcs} gtest_core.cpp)
endif()

add_executable(gtest_core
${srcs}
)
if(SGX_FOUND)
  target_include_directories(gtest_core PRIVATE "${SGX_INCLUDE_DIR}")
  target_link_directories(gtest_core PRIVATE "${SGX_LIBRARY_DIR}")
endif()

target_link_libraries(gtest_core common gtest core core_stdeth)

add_test(NAME test-core
  COMMAND gtest_core)

add_executable(gen_input_param gen_input_param.cpp)
target_link_libraries(gen_input_param core pthread)
gtest_discover_tests(gtest_core)
AddCoverage(gtest_core)

if(SGX_FOUND)
add_subdirectory(enclave)
add_untrusted_library(test_ypc_lib SHARED
  SRCS "test_ypc_module.cpp"
  EDL ./enclave/enclave.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox)

target_link_libraries(test_ypc_lib PRIVATE core_stdeth stbox_common)

add_executable(gen_sign_for_ypc gen_sign_for_ypc.cpp)
target_include_directories(gen_sign_for_ypc PRIVATE "${SGX_INCLUDE_DIR}")
target_link_directories(gen_sign_for_ypc PRIVATE "${SGX_LIBRARY_DIR}")
target_link_libraries(gen_sign_for_ypc test_ypc_lib core stbox_common)

add_dependencies(gen_sign_for_ypc test_ypc_enclave-sign)
target_link_libraries(gtest_core gtest core stbox_common test_ypc_lib)
endif()

add_executable(gtest_group main.cpp gtest_group.cpp)
target_link_libraries(gtest_group gtest core_stdeth core_gmssl core common)
gtest_discover_tests(gtest_group)
AddCoverage(gtest_group)

add_executable(gtest_secp256k1pri_kgt main.cpp gtest_secp256k1pri_kgt.cpp)
target_link_libraries(gtest_secp256k1pri_kgt gtest core_stdeth core common)
gtest_discover_tests(gtest_secp256k1pri_kgt)
AddCoverage(gtest_secp256k1pri_kgt)

add_executable(gtest_secp256k1pub_kgt main.cpp gtest_secp256k1pub_kgt.cpp)
target_link_libraries(gtest_secp256k1pub_kgt gtest core_stdeth core common)
gtest_discover_tests(gtest_secp256k1pub_kgt)
AddCoverage(gtest_secp256k1pub_kgt)

add_executable(gtest_sm2pri_kgt main.cpp gtest_sm2pri_kgt.cpp)
target_link_libraries(gtest_sm2pri_kgt gtest core_gmssl core common)
gtest_discover_tests(gtest_sm2pri_kgt)
AddCoverage(gtest_sm2pri_kgt)

add_executable(gtest_sm2pub_kgt main.cpp gtest_sm2pub_kgt.cpp)
target_link_libraries(gtest_sm2pub_kgt gtest core_gmssl core common)
gtest_discover_tests(gtest_sm2pub_kgt)
AddCoverage(gtest_sm2pub_kgt)
