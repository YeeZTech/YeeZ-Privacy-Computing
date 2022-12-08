set(T_SRCS eparser.cpp)

add_ypc_applet(person_first_match_multi_offchain
  CRYPTO stdeth
  SRCS ${T_SRCS})

if(SGX_MODE STREQUAL "Debug")
  enclave_sign(person_first_match_multi_offchain KEY enclave_private.pem
    CONFIG enclave.config.debug.xml)
else()
  enclave_sign(person_first_match_multi_offchain KEY enclave_private.pem
    CONFIG enclave.config.xml)
endif()
install(FILES ${PROJECT_SOURCE_DIR}/lib/person_first_match_multi_offchain.signed.so DESTINATION lib)
