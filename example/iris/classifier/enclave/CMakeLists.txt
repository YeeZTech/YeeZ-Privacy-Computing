set(T_SRCS eparser.cpp )

add_ypc_applet(iris_classifier
  CRYPTO stdeth
  SRCS ${T_SRCS})

if(SGX_MODE STREQUAL "Debug")
  enclave_sign(iris_classifier KEY enclave_private.pem
    CONFIG enclave.config.debug.xml)
else()
  enclave_sign(iris_classifier KEY enclave_private.pem
    CONFIG enclave.config.xml)
endif()


install(FILES ${PROJECT_SOURCE_DIR}/lib/iris_classifier.signed.so DESTINATION lib)
