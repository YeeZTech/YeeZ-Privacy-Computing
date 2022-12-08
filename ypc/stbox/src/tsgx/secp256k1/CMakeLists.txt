
add_definitions(-DUSE_NUM_NONE -DUSE_FIELD_5X52 -DHAVE___INT128 -DUSE_FIELD_INV_BUILTIN -DUSE_SCALAR_4X64 -DUSE_SCALAR_INV_BUILTIN -DUSE_LOW_HEAP)
include_directories(${PROJECT_SOURCE_DIR}/ypc/stbox/src/tsgx/secp256k1)

set(secp256k1_src ./secp256k1.c)

#add_library(stbox_secp256k1_u SHARED ${secp256k1_src})

if(SGX_FOUND)
add_sgx_library(stbox_secp256k1_t
  stbox_secp256k1_t
  SRCS "${secp256k1_src}")

install(TARGETS stbox_secp256k1_t DESTINATION lib COMPONENT c_stbox_common OPTIONAL)

target_include_directories(stbox_secp256k1_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ypc/stbox>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ypc/stbox>"
)

install(TARGETS stbox_secp256k1_t EXPORT mod_secp256k1_t
  DESTINATION lib)

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

install(TARGETS stbox_secp256k1 EXPORT mod_secp256k1
  DESTINATION lib
)

install(EXPORT mod_secp256k1
  DESTINATION "${config_install_dir}/stbox"
  NAMESPACE "${namespace}")

add_version(stbox_secp256k1)
