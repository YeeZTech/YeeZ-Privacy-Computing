
set(secp256k1_src ./secp256k1.c)

if(SGX_FOUND)
add_sgx_library(stbox_secp256k1_t
  stbox_secp256k1_t
  SRCS "${secp256k1_src}")

install(TARGETS stbox_secp256k1_t
  DESTINATION "${lib_install_dir}"
  COMPONENT c_stbox_common OPTIONAL)

target_include_directories(stbox_secp256k1_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ypc/stbox>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ypc/stbox>"
)
target_include_directories(stbox_secp256k1_t PRIVATE
  ${PROJECT_SOURCE_DIR}/ypc/stbox/src/tsgx/secp256k1
  )

target_compile_definitions(stbox_secp256k1_t PUBLIC
  USE_NUM_NONE USE_FIELD_5X52
  HAVE___INT128 USE_FIELD_INV_BUILTIN
  USE_SCALAR_4X64 USE_SCALAR_INV_BUILTIN
  USE_LOW_HEAP)

install(TARGETS stbox_secp256k1_t EXPORT mod_secp256k1_t
  DESTINATION "${lib_install_dir}")

install(EXPORT mod_secp256k1_t
  DESTINATION "${config_install_dir}/stbox"
  NAMESPACE "${namespace}")
endif()

add_library(stbox_secp256k1 SHARED ${secp256k1_src})

target_include_directories(stbox_secp256k1 PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ypc/stbox>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ypc/stbox>"
)
target_include_directories(stbox_secp256k1 PRIVATE
  ${PROJECT_SOURCE_DIR}/ypc/stbox/src/tsgx/secp256k1
)

target_compile_definitions(stbox_secp256k1 PUBLIC
  USE_NUM_NONE USE_FIELD_5X52
  HAVE___INT128 USE_FIELD_INV_BUILTIN
  USE_SCALAR_4X64 USE_SCALAR_INV_BUILTIN
)

install(TARGETS stbox_secp256k1 EXPORT mod_secp256k1
  DESTINATION "${lib_install_dir}")

install(EXPORT mod_secp256k1
  DESTINATION "${config_install_dir}/stbox"
  NAMESPACE "${namespace}")

add_version(stbox_secp256k1)
