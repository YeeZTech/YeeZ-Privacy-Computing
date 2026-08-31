find_package(OpenSSL REQUIRED)

# The plugin runtime has to exist before add_fid_parser_plugin() runs.
add_subdirectory(plugin_runtime)
add_subdirectory(plugins)

add_executable(fid_analyzer
  plugin_main.cpp
  plugin_parser.cpp
  parser_plugin.cpp
  parser_registry.cpp
  )

target_link_libraries(fid_analyzer
  stbox_common
  core
  keymgr_module
  keymgr_utils
  glog
  ${CMAKE_DL_LIBS}
  ${OPENSSL_CRYPTO_LIBRARY}
  )
target_include_directories(fid_analyzer PRIVATE
  "${PROJECT_SOURCE_DIR}/toolkit/analyzer/"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "${OPENSSL_INCLUDE_DIR}"
  )
target_compile_definitions(fid_analyzer PRIVATE
  FID_DEFAULT_PARSER_REGISTRY_DIR="${CMAKE_INSTALL_PREFIX}/share/fidelius/parser-registry.d")
set_target_properties(fid_analyzer PROPERTIES
  INSTALL_RPATH "\$ORIGIN/../lib"
  LINK_FLAGS "-Wl,--disable-new-dtags")


install(TARGETS fid_analyzer
  DESTINATION "${bin_install_dir}")

AddClangTidy(fid_analyzer)
EnableCoverage(fid_analyzer)
