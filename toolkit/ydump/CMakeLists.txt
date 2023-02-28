add_executable(ydump
  main.cpp
  nouse_bridge.cpp)
target_link_libraries(ydump core keymgr_utils keymgr_module core_parser_module stbox_common)
install(TARGETS ydump
  DESTINATION "${bin_install_dir}")

AddClangTidy(ydump)
