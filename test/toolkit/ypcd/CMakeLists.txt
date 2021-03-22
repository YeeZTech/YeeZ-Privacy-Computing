
add_executable(gtest_toolkit_ypcd
  gtest_db.cpp
  gtest_services.cpp
  main.cpp
  )
target_link_libraries(gtest_toolkit_ypcd gtest ypc_core libypcd)

add_test(NAME test-toolkit-ypcd
  COMMAND gtest_toolkit_ypcd)
