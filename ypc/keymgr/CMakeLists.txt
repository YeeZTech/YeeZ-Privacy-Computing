
add_subdirectory(common)
if(SGX_FOUND)
  add_subdirectory(default)
endif()

