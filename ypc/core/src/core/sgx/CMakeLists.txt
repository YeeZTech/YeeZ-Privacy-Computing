set(srcs parser_sgx_module.cpp
  )
add_untrusted_library(core_parser_module SHARED
  SRCS ${srcs}
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/eparser.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox/)

target_link_libraries(core_parser_module PUBLIC stbox_common)
#install(TARGETS ypc_parser_module DESTINATION lib COMPONENT c_ypc_core OPTIONAL)

target_include_directories(core_parser_module PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_include_directories(core_parser_module INTERFACE
  "$<INSTALL_INTERFACE:${SGX_INCLUDE_DIR}>")
target_link_directories(core_parser_module INTERFACE
  "$<INSTALL_INTERFACE:${SGX_LIBRARY_DIR}>")

install(TARGETS core_parser_module EXPORT mod_core_parser_module
  DESTINATION "${lib_install_dir}"
  COMPONENT core)
install(EXPORT mod_core_parser_module
  DESTINATION "${config_install_dir}/core"
  NAMESPACE "${namespace}"
  COMPONENT core
  )

AddClangTidy(core_parser_module)
add_version(core_parser_module)
