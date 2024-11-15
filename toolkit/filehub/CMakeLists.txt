add_executable(data_provider main.cpp )
target_link_libraries(data_provider stbox_common core core_stdeth core_gmssl)
install(TARGETS data_provider
  DESTINATION "${bin_install_dir}")
target_include_directories(data_provider PRIVATE
"${PROJECT_SOURCE_DIR}/toolkit/datahub/"
)
AddClangTidy(data_provider)
EnableCoverage(data_provider)

add_executable(unseal_dir unseal_dir.cpp )
target_link_libraries(unseal_dir stbox_common core core_stdeth core_gmssl)
install(TARGETS unseal_dir
  DESTINATION "${bin_install_dir}")
AddClangTidy(unseal_dir)
EnableCoverage(unseal_dir)
