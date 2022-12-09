
set(src keccak.c)
add_library(stbox_keccak SHARED ${src})

target_include_directories(stbox_keccak PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

install(TARGETS stbox_keccak EXPORT mod_keccak
  DESTINATION lib
)

install(EXPORT mod_keccak
  DESTINATION "${config_install_dir}/stbox"
  NAMESPACE "${namespace}")

add_version(stbox_keccak)

if(SGX_FOUND)
add_sgx_library(stbox_keccak_t SRCS ${src})

target_include_directories(stbox_keccak_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

install(TARGETS stbox_keccak_t EXPORT mod_keccak_t
  DESTINATION lib
)

install(EXPORT mod_keccak_t
  DESTINATION "${config_install_dir}/stbox"
  NAMESPACE "${namespace}")
endif()
