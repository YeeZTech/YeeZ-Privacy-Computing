set(T_SRCS eparser.cpp)

add_ypc_applet(iris_parser
  CRYPTO stdeth
  SRCS ${T_SRCS}
  )

#target_link_libraries(iris_parser stbox_common_t stbox_channel_t analyzer_t stdeth_t gmssl_t)

if(SGX_MODE STREQUAL "Debug")
  enclave_sign(iris_parser KEY enclave_private.pem
    CONFIG enclave.config.debug.xml)
else()
  enclave_sign(iris_parser KEY enclave_private.pem
    CONFIG enclave.config.xml)
endif()
