add_executable(data_provider main.cpp )
target_link_libraries(data_provider stbox_common core core_stdeth core_gmssl)
install(TARGETS data_provider
  DESTINATION "${bin_install_dir}")
AddClangTidy(data_provider)
EnableCoverage(data_provider)

# add_executable(oram_data_provider oram_seal_file.cpp )
# target_link_libraries(oram_data_provider stbox_common core core_stdeth core_gmssl)
# install(TARGETS oram_data_provider
#   DESTINATION "${bin_install_dir}")
# AddClangTidy(oram_data_provider)
# EnableCoverage(oram_data_provider)