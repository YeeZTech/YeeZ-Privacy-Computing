add_executable(data_provider main.cpp )
target_link_libraries(data_provider stbox_common core core_stdeth core_gmssl)
install(TARGETS data_provider
  DESTINATION "${bin_install_dir}")
AddClangTidy(data_provider)
EnableCoverage(data_provider)

add_executable(unseal_file unseal.cpp )
target_link_libraries(unseal_file stbox_common core core_stdeth core_gmssl)
install(TARGETS unseal_file
  DESTINATION "${bin_install_dir}")
AddClangTidy(unseal_file)
EnableCoverage(unseal_file)
