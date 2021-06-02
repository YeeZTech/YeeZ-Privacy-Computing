set(src byte/base58.cpp byte/base64.cpp)

add_sgx_library(ypc_common_t
  ypc_common_t
  SRCS ${src})
install(TARGETS ypc_common_t DESTINATION lib)

add_library(ypc_common_u SHARED ${src})
install(TARGETS ypc_common_u DESTINATION lib)
