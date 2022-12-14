add_library(keymgr_utils SHARED
  util.cpp)

target_include_directories(keymgr_utils PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )

target_link_libraries(keymgr_utils core)

install(TARGETS keymgr_utils
  EXPORT mod_keymgr_utils
  DESTINATION lib )

install(EXPORT mod_keymgr_utils
  DESTINATION "${config_install_dir}/keymgr"
  NAMESPACE "${namespace}"
  )

add_version(keymgr_utils)
AddClangTidy(keymgr_utils)
EnableCoverage(keymgr_utils)
