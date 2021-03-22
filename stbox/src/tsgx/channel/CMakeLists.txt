set(t_channel_src
  dh_session.cpp
  dh_session_responder.cpp
  dh_session_initiator.cpp
  #tsgx/channel/util.cpp
)

add_trusted_library(stbox_channel_t
  stbox_channel
  SRCS "${t_channel_src}"
  EDL ./dh_channel.edl
  EDL_SEARCH_PATHS .)

add_untrusted_library(stbox_channel_u
  STATIC
  SRCS ""
  EDL ./dh_channel.edl
  EDL_SEARCH_PATHS ./)

target_link_libraries(stbox_channel_t stbox_common_t)
target_link_libraries(stbox_channel_u stbox_common_u)


