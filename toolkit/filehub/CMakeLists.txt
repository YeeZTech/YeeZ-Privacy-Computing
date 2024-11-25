add_executable(file_provider main.cpp )
target_link_libraries(file_provider stbox_common core core_stdeth core_gmssl)
install(TARGETS file_provider
  DESTINATION "${bin_install_dir}")
target_include_directories(file_provider PRIVATE
"${PROJECT_SOURCE_DIR}/toolkit/filehub/"
)
AddClangTidy(file_provider)
EnableCoverage(file_provider)

add_executable(unseal_dir unseal_dir.cpp )
target_link_libraries(unseal_dir stbox_common core core_stdeth core_gmssl)
install(TARGETS unseal_dir
  DESTINATION "${bin_install_dir}")
AddClangTidy(unseal_dir)
EnableCoverage(unseal_dir)
