set(T_SRCS eparser.cpp)

add_ypc_applet(convert_sealed_file2
  CRYPTO stdeth
  SRCS ${T_SRCS})

if(SGX_MODE STREQUAL "Debug")
  enclave_sign(convert_sealed_file2 KEY enclave_private.pem
    CONFIG enclave.config.debug.xml)
else()
  enclave_sign(convert_sealed_file2 KEY enclave_private.pem
    CONFIG enclave.config.xml)
endif()
