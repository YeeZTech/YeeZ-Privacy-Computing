add_executable(data_provider main.cpp )
target_link_libraries(data_provider stbox_common core core_stdeth core_gmssl)
install(TARGETS data_provider
  DESTINATION "${bin_install_dir}")

add_library(data_provider_lib lib.cpp)
set_property(TARGET data_provider_lib PROPERTY OUTPUT_NAME data_provider)
#set_target_properties(data_provider_lib PROPERTIES
#        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
target_link_libraries(data_provider_lib stbox_common core core_stdeth core_gmssl)
install(TARGETS data_provider_lib
  DESTINATION "${lib_install_dir}")

AddClangTidy(data_provider)
EnableCoverage(data_provider)
