add_executable(gtest_terminus
  main.cpp
  gtest_intel_sgx.cpp
  )
target_link_libraries(gtest_terminus gtest ypc_terminus glog boost_system pthread)

add_test(NAME gtest-terminus
  COMMAND gtest_terminus)
