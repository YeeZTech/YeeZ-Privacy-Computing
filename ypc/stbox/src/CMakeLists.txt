set(stbox_src stx_status.cpp stx_common.cpp exception.cpp)
set(stbox_t_src ${stbox_src}
  ./tsgx/stx_common_trusted.cpp
  ./tsgx/log.cpp
  ./tsgx/util.cpp)

if(SGX_FOUND)
add_trusted_library(stbox_common_t
  #stbox_common_t
  SRCS "${stbox_t_src}"
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/stbox/stbox.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/stbox/)

target_include_directories(stbox_common_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ypc/stbox>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ypc/stbox>"
)

install(TARGETS stbox_common_t EXPORT mod_stbox_common_t
  DESTINATION lib)

install(EXPORT mod_stbox_common_t
  DESTINATION "${config_install_dir}/stbox"
  NAMESPACE "${namespace}")

set(stbox_core_src ${stbox_src} usgx/error_message.cpp
  usgx/sgx_module.cpp
  usgx/stx_common_untrusted.cpp
  )

add_untrusted_library(stbox_common SHARED
  SRCS "${stbox_core_src}"
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/stbox/stbox.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/stbox/)


else()
  add_library(stbox_common SHARED
    stx_status.cpp
    stx_common.cpp
    usgx/stx_common_untrusted.cpp)
endif()

target_include_directories(stbox_common PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ypc/stbox>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ypc/stbox>"
)

install(TARGETS stbox_common EXPORT mod_stbox_common
  DESTINATION lib)

install(EXPORT mod_stbox_common
  DESTINATION "${config_install_dir}/stbox"
  NAMESPACE "${namespace}")

add_version(stbox_common)

add_subdirectory(gmssl)
add_subdirectory(keccak)
if(SGX_FOUND)
add_subdirectory(tsgx)
endif()
