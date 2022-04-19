add_executable(data_provider main.cpp )
target_link_libraries(data_provider stbox_common_u ypc_core ${ypc_dep_libs} ypc_stdeth_u)
install(TARGETS data_provider DESTINATION bin COMPONENT c_datahub OPTIONAL)
