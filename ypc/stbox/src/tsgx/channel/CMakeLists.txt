set(t_channel_src
  dh_session.cpp
  dh_session_responder.cpp
  dh_session_initiator.cpp
)

add_sgx_library(stbox_channel_t
  stbox_channel
  SRCS "${t_channel_src}"
)

target_link_libraries(stbox_channel_t stbox_common_t)

target_include_directories(stbox_channel_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ypc/stbox>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ypc/stbox>"
)

install(TARGETS stbox_channel_t EXPORT mod_channel_t
  DESTINATION lib)

install(EXPORT mod_channel_t
  DESTINATION "${config_install_dir}/stbox"
  NAMESPACE "${namespace}")
