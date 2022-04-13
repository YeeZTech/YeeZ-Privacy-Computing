
add_definitions(-DUSE_NUM_NONE -DUSE_FIELD_5X52 -DHAVE___INT128 -DUSE_FIELD_INV_BUILTIN -DUSE_SCALAR_4X64 -DUSE_SCALAR_INV_BUILTIN -DUSE_LOW_HEAP)
include_directories(${PROJECT_SOURCE_DIR}/stbox/src/tsgx/secp256k1)

set(secp256k1_src ./secp256k1.c)

#add_library(stbox_secp256k1_u SHARED ${secp256k1_src})

add_sgx_library(stbox_secp256k1_t
  stbox_secp256k1_t
  SRCS "${secp256k1_src}")

#install(TARGETS stbox_secp256k1_t DESTINATION lib COMPONENT c_stbox_common OPTIONAL)
