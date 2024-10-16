set(T_SRCS eparser.cpp)

#add_ypc_applet(iris_parser
  #CRYPTO stdeth
  #SRCS ${T_SRCS}
  #)

add_enclave_library(iris_parser SRCS ${T_SRCS}
  EDL iris.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/edl/util/:${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox/
)
target_include_directories(iris_parser-edlobj PUBLIC
  "${PROJECT_SOURCE_DIR}/include"
  )
target_include_directories(iris_parser PUBLIC
  "${PROJECT_SOURCE_DIR}/include"
  )
target_link_libraries(iris_parser PRIVATE
  stbox_common_t
  stbox_channel_t
  analyzer_t stdeth_t)

if(SGX_MODE STREQUAL "Debug")
  enclave_sign(iris_parser KEY enclave_private.pem
    CONFIG enclave.config.debug.xml)
else()
  enclave_sign(iris_parser KEY enclave_private.pem
    CONFIG enclave.config.xml)
endif()
