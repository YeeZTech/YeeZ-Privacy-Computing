
add_definitions(-DUSE_NUM_NONE -DUSE_FIELD_5X52 -DHAVE___INT128 -DUSE_FIELD_INV_BUILTIN -DUSE_SCALAR_4X64 -DUSE_SCALAR_INV_BUILTIN)
include_directories(${PROJECT_SOURCE_DIR}/stbox/src/tsgx/secp256k1)

set(secp256k1_src ./secp256k1.c)

add_sgx_library(stbox_secp256k1_t
  stbox_secp256k1_t
  SRCS "${secp256k1_src}")
  #EDL ./secp256k1.edl
  #EDL_SEARCH_PATHS .)
install(TARGETS stbox_secp256k1_t DESTINATION lib)
