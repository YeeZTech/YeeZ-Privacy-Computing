set(T_SRCS eparser.cpp)

add_ypc_applet(person_first_match_multi
  CRYPTO stdeth
  SRCS ${T_SRCS})

if(SGX_MODE STREQUAL "Debug")
  enclave_sign(person_first_match_multi KEY enclave_private.pem
    CONFIG enclave.config.debug.xml)
else()
  enclave_sign(person_first_match_multi KEY enclave_private.pem
    CONFIG enclave.config.xml)
endif()
install(FILES ${PROJECT_SOURCE_DIR}/lib/person_first_match_multi.signed.so DESTINATION lib)
