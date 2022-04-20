set(T_SRCS eparser.cpp)

add_enclave_library(person_first_match SRCS ${T_SRCS}
  EDL ${PROJECT_SOURCE_DIR}/core/edl/eparser.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/core/edl/:${PROJECT_SOURCE_DIR}/stbox/src/:${PROJECT_SOURCE_DIR}/stbox/src/tsgx/crypto/
  )

target_link_libraries(person_first_match stbox_common_t stbox_channel_t ypc_analyzer_t ypc_stdeth_t)


enclave_sign(person_first_match KEY enclave_private.pem
  CONFIG enclave.config.xml)
install(FILES ${PROJECT_SOURCE_DIR}/lib/person_first_match.signed.so DESTINATION lib)
