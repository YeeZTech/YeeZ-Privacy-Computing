set(src byte/base58.cpp byte/base64.cpp version.cpp)

if(SGX_FOUND)
add_sgx_library(common_t
  common_t
  SRCS ${src})

target_include_directories(common_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

install(TARGETS common_t EXPORT mod_common_t
  DESTINATION lib
  )

install(EXPORT mod_common_t
  DESTINATION "${config_install_dir}/common"
  NAMESPACE "${namespace}")
endif()

add_library(common SHARED ${src})

target_include_directories(common PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )

install(TARGETS common EXPORT mod_common
  DESTINATION lib
  )

install(EXPORT mod_common
  DESTINATION "${config_install_dir}/common"
  NAMESPACE "${namespace}")

add_version(common)

