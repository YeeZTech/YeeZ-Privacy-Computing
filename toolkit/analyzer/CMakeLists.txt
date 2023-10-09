#include_directories(${PROJECT_SOURCE_DIR}/toolkit/analyzer/)
add_executable(fid_analyzer main.cpp
  sgx_bridge.cpp
  #parsers/parser.cpp
  parsers/tg_parser.cpp
  )

target_link_libraries(fid_analyzer
  stbox_common
  core
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

add_library(fid_analyzer_lib 
  lib.cpp
  sgx_bridge.cpp
  #parsers/parser.cpp
  parsers/tg_parser.cpp)
set_property(TARGET fid_analyzer_lib PROPERTY OUTPUT_NAME fid_analyzer)
#set_target_properties(fid_analyzer_lib PROPERTIES
#        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
target_link_libraries(fid_analyzer_lib 
  stbox_common
  core
  core_parser_module
  keymgr_module
  keymgr_utils
  glog
  )
target_include_directories(fid_analyzer_lib PRIVATE
  "${PROJECT_SOURCE_DIR}/toolkit/analyzer/"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  )
install(TARGETS fid_analyzer_lib
  DESTINATION "${lib_install_dir}")

AddClangTidy(fid_analyzer)
EnableCoverage(fid_analyzer)
