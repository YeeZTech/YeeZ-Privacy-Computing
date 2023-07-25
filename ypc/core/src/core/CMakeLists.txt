if(SGX_FOUND)
add_subdirectory(sgx)
endif()

add_subdirectory(crypto)
find_package (Threads REQUIRED)

add_library(core SHARED
  poption_require.cpp
  db.cpp
  configuration.cpp
  sealed_file.cpp
  oram_sealed_file.cpp
  command_executor.cpp
  filesystem.cpp
  privacy_data_reader.cpp
  exceptions.cpp
  status.cpp
  version.cpp
  ${FF_PATH}/src/net/common/archive.cpp)

target_include_directories(core PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_libraries(core PUBLIC
  mysqlcppconn
  boost_program_options
  boost_filesystem
  glog
  boost_system
  ${CMAKE_THREAD_LIBS_INIT}
  common
  )
#target_link_libraries(core INTERFACE stbox_common)
#target_link_libraries(core INTERFACE core_parser_module)

install(TARGETS core EXPORT mod_core
  DESTINATION "${lib_install_dir}"
  COMPONENT core)

install(EXPORT mod_core
  DESTINATION "${config_install_dir}/core"
  NAMESPACE "${namespace}"
  COMPONENT core
  )
AddClangTidy(core)
EnableCoverage(core)
add_version(core)

