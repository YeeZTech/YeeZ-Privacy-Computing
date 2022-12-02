add_executable(ydump
  main.cpp
  nouse_bridge.cpp)
target_link_libraries(ydump core stbox_common keymgr_utils keymgr_module core_parser_module )
install(TARGETS ydump DESTINATION bin)

AddClangTidy(ydump)
