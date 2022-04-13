set(t_channel_src
  dh_session.cpp
  dh_session_responder.cpp
  dh_session_initiator.cpp
  #tsgx/channel/util.cpp
)

add_sgx_library(stbox_channel_t
  stbox_channel
  SRCS "${t_channel_src}"
)
target_link_libraries(stbox_channel_t stbox_common_t)
#install(TARGETS stbox_channel_t DESTINATION lib COMPONENT c_stbox_common OPTIONAL)
