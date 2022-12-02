add_executable(gtest_terminus
  main.cpp
  gtest_intel_sgx.cpp
  )
find_package (Threads REQUIRED)

target_include_directories(gtest_terminus PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )
target_link_libraries(gtest_terminus gtest terminus
  ${CMAKE_THREAD_LIBS_INIT}
  boost_system
  )

gtest_discover_tests(gtest_terminus)
