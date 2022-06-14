set(src byte/base58.cpp byte/base64.cpp)

add_sgx_library(ypc_common_t
  ypc_common_t
  SRCS ${src})
# trusted library is staticly linked, do not need to install
#install(TARGETS ypc_common_t DESTINATION lib COMPONENT c_ypc_common OPTIONAL)

add_library(ypc_common_u SHARED ${src})
install(TARGETS ypc_common_u DESTINATION lib COMPONENT c_ypc_common OPTIONAL)
