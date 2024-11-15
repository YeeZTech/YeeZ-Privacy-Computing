add_subdirectory(modules)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

add_executable(fid_analyzer dianshu_analyzer.cpp
  dianshu_parser.cpp
  )

target_link_libraries(fid_analyzer
  stbox_common
  core
  core_parser_module
  keymgr_module
  keymgr_utils
  glog
  dl
  )
target_include_directories(fid_analyzer PRIVATE
  "${PROJECT_SOURCE_DIR}/toolkit/analyzer/"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  )

install(TARGETS fid_analyzer
  DESTINATION "${bin_install_dir}")
AddClangTidy(fid_analyzer)
EnableCoverage(fid_analyzer)
