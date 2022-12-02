set(src
  crypto_pack.cpp
  single_data_onchain_result.cpp
  enclave_interaction.cpp
)

add_library(terminus SHARED ${src})

target_link_libraries(terminus core_stdeth core_gmssl glog)

target_include_directories(terminus PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )

install(TARGETS terminus EXPORT mod_terminus
  DESTINATION lib)
install(EXPORT mod_terminus
  DESTINATION "${config_install_dir}/toolkit"
  NAMESPACE "${namespace}"
  )

AddClangTidy(terminus)
