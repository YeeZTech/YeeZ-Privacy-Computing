add_executable(gtest_core
  main.cpp
  gtest_common.cpp
  gtest_byte.cpp
  gtest_base58.cpp
  gtest_base64.cpp
  gtest_sha.cpp
  gtest_command_executor.cpp
  gtest_filesystem.cpp
  gtest_configuration.cpp
  gtest_core.cpp
  gtest_blockfile.cpp
  gtest_sealed_file.cpp
  gtest_ntobject_file.cpp)
target_link_libraries(gtest_core gtest ypc_core stbox_common_u)

add_test(NAME test-core
  COMMAND gtest_core)
