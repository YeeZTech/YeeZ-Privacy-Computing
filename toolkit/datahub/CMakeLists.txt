add_executable(data_provider main.cpp )
target_link_libraries(data_provider
  PUBLIC core
  PRIVATE stbox_common core_stdeth core_gmssl)
install(TARGETS data_provider
  DESTINATION "${bin_install_dir}")
AddClangTidy(data_provider)
EnableCoverage(data_provider)
