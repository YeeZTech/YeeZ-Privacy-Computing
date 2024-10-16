#include_directories(${PROJECT_SOURCE_DIR}/toolkit/analyzer/)
add_subdirectory(modules)
add_executable(fid_analyzer main.cpp
  sgx_bridge.cpp
  )

target_link_libraries(fid_analyzer
  stbox_common
  core
  iris_parser_module
  core_parser_module
  keymgr_module
  keymgr_utils
  glog
  )
target_include_directories(fid_analyzer PRIVATE
  "${PROJECT_SOURCE_DIR}/toolkit/analyzer/"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  )

install(TARGETS fid_analyzer
  DESTINATION "${bin_install_dir}")
AddClangTidy(fid_analyzer)
EnableCoverage(fid_analyzer)
