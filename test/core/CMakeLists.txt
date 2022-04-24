add_subdirectory(enclave)

add_executable(gtest_core
  main.cpp
  gtest_common.cpp
  gtest_byte.cpp
  gtest_base58.cpp
  gtest_base64.cpp
  gtest_command_executor.cpp
  gtest_filesystem.cpp
  gtest_configuration.cpp
  gtest_core.cpp
  gtest_blockfile.cpp
  gtest_sealed_file.cpp
  gtest_ntobject_file.cpp
  gtest_ntjson.cpp
  )

add_test(NAME test-core
  COMMAND gtest_core)

set(edl_path ${PROJECT_SOURCE_DIR}/core/edl/:${PROJECT_SOURCE_DIR}/stbox/src)


add_untrusted_library(test_ypc_lib SHARED
  SRCS "test_ypc_module.cpp"
  EDL ./enclave/enclave.edl
  EDL_SEARCH_PATHS ${edl_path})
add_executable(gen_sign_for_ypc gen_sign_for_ypc.cpp)
target_link_libraries(gen_sign_for_ypc test_ypc_lib ypc_core stbox_common_u)

add_dependencies(gen_sign_for_ypc test_ypc_enclave-sign)
target_link_libraries(gtest_core gtest ypc_core stbox_common_u test_ypc_lib)

add_executable(gen_input_param gen_input_param.cpp)
target_link_libraries(gen_input_param ypc_core pthread)
