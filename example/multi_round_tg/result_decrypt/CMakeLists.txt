add_executable(result_decrypt main.cpp )
target_link_libraries(result_decrypt core core_stdeth core_gmssl)
target_include_directories(result_decrypt PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ypc/stbox>"
  )
