add_executable(data_provider main.cpp )
target_link_libraries(data_provider stbox_common core core_stdeth core_gmssl)
install(TARGETS data_provider DESTINATION bin)
