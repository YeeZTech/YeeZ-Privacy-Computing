add_executable(yprepare
  main.cpp
  onchain_data_reader.cpp
  nouse_bridge.cpp)
install(TARGETS yprepare DESTINATION bin)

target_link_libraries(yprepare ypc_core stbox_common_u keymgr_utils keymgr_module ypc_parser_module )
