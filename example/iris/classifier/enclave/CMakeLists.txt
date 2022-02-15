set(T_SRCS eparser.cpp )

add_enclave_library(iris_classifier SRCS ${T_SRCS}
  EDL ${PROJECT_SOURCE_DIR}/core/edl/eparser.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/core/edl/:${PROJECT_SOURCE_DIR}/stbox/src/
  )

target_link_libraries(iris_classifier stbox_common_t stbox_channel_t ypc_analyzer_t ypc_stdeth_t)


enclave_sign(iris_classifier KEY enclave_private.pem
  CONFIG enclave.config.xml)
install(FILES ${PROJECT_SOURCE_DIR}/lib/iris_classifier.signed.so DESTINATION lib)
