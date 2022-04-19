set(src
  sm_compatible.cpp
  sgx_and_eth_compatible/crypto_pack.cpp
  single_data_onchain_result.cpp
  enclave_interaction.cpp
)

add_library(ypc_terminus SHARED ${src})
target_link_libraries(ypc_terminus ypc_stdeth_u)
install(TARGETS ypc_terminus DESTINATION lib COMPONENT c_yterminus OPTIONAL)
