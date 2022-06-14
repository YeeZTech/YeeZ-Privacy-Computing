
add_library(stbox_keccak_u keccak/keccak.c)
add_sgx_library(stbox_keccak_t stbox_keccak_t SRCS keccak/keccak.c)

set(stbox_src stx_status.cpp stx_common.cpp exception.cpp eth/eth_hash.cpp eth/util.cpp)


set(stbox_t_src ${stbox_src}
  ./tsgx/stx_common_trusted.cpp
  ./tsgx/log.cpp
  ./tsgx/util.cpp)

add_trusted_library(stbox_common_t
  stbox_common
  SRCS "${stbox_t_src}"
  EDL ./stbox.edl
  EDL_SEARCH_PATHS .)
# trusted library is staticly linked, do not need to install
#install(TARGETS stbox_common_t DESTINATION lib COMPONENT c_stbox_common OPTIONAL)

set(stbox_core_src ${stbox_src} usgx/error_message.cpp
  usgx/sgx_module.cpp
  usgx/stx_common_untrusted.cpp
  )

add_untrusted_library(stbox_common_u SHARED
  SRCS "${stbox_core_src}"
  EDL ./stbox.edl
  EDL_SEARCH_PATHS .)
target_link_libraries(stbox_common_u glog stbox_keccak_u)
target_link_libraries(stbox_common_t stbox_keccak_t)
install(TARGETS stbox_common_u DESTINATION lib COMPONENT c_stbox_common OPTIONAL)


add_subdirectory(tsgx)
