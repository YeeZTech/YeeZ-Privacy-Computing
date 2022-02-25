add_executable(ydump
  main.cpp
  nouse_bridge.cpp)
install(TARGETS ydump DESTINATION bin COMPONENT ydumpp OPTIONAL)
target_link_libraries(ydump ypc_core stbox_common_u keymgr_utils keymgr_module ypc_parser_module )
