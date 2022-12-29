set(T_SRCS eparser.cpp)

add_ypc_applet(iris_parser
  CRYPTO stdeth
  SRCS ${T_SRCS}
  )

if(SGX_MODE STREQUAL "Debug")
  enclave_sign(iris_parser KEY enclave_private.pem
    CONFIG enclave.config.debug.xml)
else()
  enclave_sign(iris_parser KEY enclave_private.pem
    CONFIG enclave.config.xml)
endif()
