set(src byte/base58.cpp byte/base64.cpp version.cpp)

if(SGX_FOUND)
add_sgx_library(common_t
  common_t
  SRCS ${src})

target_include_directories(common_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

install(TARGETS common_t EXPORT mod_common_t
  DESTINATION "${lib_install_dir}"
  COMPONENT common_t)

install(EXPORT mod_common_t
  DESTINATION "${config_install_dir}/common"
  NAMESPACE "${namespace}"
  COMPONENT common_t)
endif()

add_library(common SHARED ${src})

target_include_directories(common PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )

install(TARGETS common EXPORT mod_common
  DESTINATION "${lib_install_dir}"
  COMPONENT common)

install(EXPORT mod_common
  DESTINATION "${config_install_dir}/common"
  NAMESPACE "${namespace}"
  COMPONENT common)

add_version(common)

