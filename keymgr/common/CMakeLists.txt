add_library(keymgr_utils EXCLUDE_FROM_ALL SHARED
  util.cpp)
target_link_libraries(keymgr_utils ypc_core)
install(TARGETS keymgr_utils DESTINATION lib COMPONENT keymgr OPTIONAL)
