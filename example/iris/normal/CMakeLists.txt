add_executable(iris_normal
  main.cpp
  ../analyzer/enclave/iris_parser.cpp
  ../analyzer/enclave/sgx_plugin.cpp
  )

target_link_libraries(iris_normal ypc_core stbox_common_u ff_net hpda)
