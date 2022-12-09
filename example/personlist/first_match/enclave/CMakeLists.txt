set(T_SRCS eparser.cpp)

add_ypc_applet(person_first_match
  CRYPTO stdeth
  SRCS ${T_SRCS})

if(SGX_MODE STREQUAL "Debug")
  enclave_sign(person_first_match KEY enclave_private.pem
    CONFIG enclave.config.debug.xml)
else()
  enclave_sign(person_first_match KEY enclave_private.pem
    CONFIG enclave.config.xml)
endif()
