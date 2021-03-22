set(stbox_src stx_status.cpp stx_common.cpp exception.cpp ebyte.cpp eth/eth_hash.cpp eth/util.cpp keccak/keccak.c)


set(stbox_t_src ${stbox_src}
  ./tsgx/stx_common_trusted.cpp
  ./tsgx/log.cpp)

add_trusted_library(stbox_common_t
  stbox_common
  SRCS "${stbox_t_src}"
  EDL ./stbox.edl
  EDL_SEARCH_PATHS .)

set(stbox_core_src ${stbox_src} usgx/error_message.cpp
  usgx/sgx_module.cpp
  usgx/stx_common_untrusted.cpp
  )

add_untrusted_library(stbox_common_u SHARED
  SRCS "${stbox_core_src}"
  EDL ./stbox.edl
  EDL_SEARCH_PATHS .)

target_link_libraries(stbox_common_u glog)


add_subdirectory(tsgx)
