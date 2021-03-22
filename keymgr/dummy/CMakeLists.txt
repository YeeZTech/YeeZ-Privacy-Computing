
add_subdirectory(enclave)

set(srcs
  App.cpp
  UntrustedEnclaveMessageExchange.cpp
  dummy_forward.cpp
  dummy_sgx_module.cpp)
add_untrusted_executable(AppInitiator
  SRCS ${srcs}
  EDL ./enclave/EnclaveInitiator.edl
  EDL_SEARCH_PATHS ./enclave)

add_dependencies(AppInitiator dummy_initiator-sign )
target_link_libraries(AppInitiator keymgr_module stbox_common_u keymgr_utils)

