
#add_definitions(-DSTBOX_CRYPTO_VERBOSE)

add_trusted_library(stbox_crypto_t
  stbox_crypto
  SRCS ecc.cpp ecp.cpp
  EDL ./ecc.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/stbox/src/)

target_link_libraries(stbox_crypto_t stbox_common_t stbox_secp256k1_t)
