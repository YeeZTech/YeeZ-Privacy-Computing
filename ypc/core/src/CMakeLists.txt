add_subdirectory(core)
if(SGX_FOUND)
add_subdirectory(core_t)
endif()
