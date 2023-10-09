add_executable(ydump
  main.cpp
  nouse_bridge.cpp)
target_link_libraries(ydump core stbox_common keymgr_utils keymgr_module core_parser_module )
install(TARGETS ydump
  DESTINATION "${bin_install_dir}")

add_library(ydump_lib
  lib.cpp
  nouse_bridge.cpp)
set_property(TARGET ydump_lib PROPERTY OUTPUT_NAME ydump)
#set_target_properties(ydump_lib PROPERTIES
#        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
target_link_libraries(ydump_lib core stbox_common keymgr_utils keymgr_module core_parser_module)
install(TARGETS ydump_lib
  DESTINATION "${lib_install_dir}")

AddClangTidy(ydump)
