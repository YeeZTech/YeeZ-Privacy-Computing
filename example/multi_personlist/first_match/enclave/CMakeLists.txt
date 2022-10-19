set(T_SRCS eparser.cpp)

add_enclave_library(person_first_match_multi SRCS ${T_SRCS}
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/eparser.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox/
  )

target_link_libraries(person_first_match_multi stbox_common_t stbox_channel_t ypc_analyzer_t ypc_stdeth_t)


enclave_sign(person_first_match_multi KEY enclave_private.pem
  CONFIG enclave.config.xml)
install(FILES ${PROJECT_SOURCE_DIR}/lib/person_first_match_multi.signed.so DESTINATION lib)
