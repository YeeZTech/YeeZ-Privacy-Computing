set(T_SRCS eparser.cpp )

add_ypc_applet(iris_parser_for_offchain
  CRYPTO stdeth
  SRCS ${T_SRCS}
  )

enclave_sign(iris_parser_for_offchain KEY enclave_private.pem
  CONFIG enclave.config.xml)
