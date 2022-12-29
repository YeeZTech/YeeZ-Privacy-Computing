

add_ypc_applet(file_operator
  CRYPTO stdeth
  SRCS enclave_basic.cpp)

enclave_sign(file_operator KEY enclave_private.pem
  CONFIG enclave.config.xml)

add_ypc_applet(file_operator_read
  CRYPTO stdeth
  SRCS enclave_read.cpp)

enclave_sign(file_operator_read KEY enclave_private.pem
  CONFIG enclave.config.xml)

add_ypc_applet(file_operator_write
  CRYPTO stdeth
  SRCS enclave_write.cpp)

enclave_sign(file_operator_write KEY enclave_private.pem
  CONFIG enclave.config.xml)
