add_subdirectory(src)
if(SGX_MODE STREQUAL "Debug")
  add_subdirectory(test)
endif()
