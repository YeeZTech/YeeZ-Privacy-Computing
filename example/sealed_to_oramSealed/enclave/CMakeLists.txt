set(T_SRCS eparser.cpp)

add_ypc_applet(transform_sealed_file
  CRYPTO stdeth
  SRCS ${T_SRCS})

if(SGX_MODE STREQUAL "Debug")
  enclave_sign(transform_sealed_file KEY enclave_private.pem
    CONFIG enclave.config.debug.xml)
else()
  enclave_sign(transform_sealed_file KEY enclave_private.pem
    CONFIG enclave.config.xml)
endif()
