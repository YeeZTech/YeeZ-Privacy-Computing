
set(T_SRCS EnclaveInitiator.cpp
  )

add_enclave_library(dummy_initiator
  SRCS ${T_SRCS}
  EDL EnclaveInitiator.edl
  EDL_SEARCH_PATHS .)

target_link_libraries(dummy_initiator stbox_common_t stbox_channel_t)

enclave_sign(dummy_initiator KEY EnclaveInitiator_private.pem
  CONFIG EnclaveInitiator.config.xml)
