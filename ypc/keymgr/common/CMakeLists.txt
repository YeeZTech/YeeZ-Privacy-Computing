add_library(keymgr_utils SHARED
  util.cpp)

target_include_directories(keymgr_utils PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )

target_link_libraries(keymgr_utils PUBLIC core)

install(TARGETS keymgr_utils
  EXPORT mod_keymgr_utils
  DESTINATION "${lib_install_dir}"
  COMPONENT keymgr)

install(EXPORT mod_keymgr_utils
  DESTINATION "${config_install_dir}/keymgr"
  NAMESPACE "${namespace}"
  COMPONENT keymgr
  )

add_version(keymgr_utils)
AddClangTidy(keymgr_utils)
EnableCoverage(keymgr_utils)
