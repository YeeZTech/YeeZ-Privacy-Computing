set(T_SRCS eparser.cpp us_covid19_parser.cpp sgx_plugin.cpp)

add_enclave_library(us_covid19_parser SRCS ${T_SRCS}
  EDL ${PROJECT_SOURCE_DIR}/core/edl/eparser.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/core/edl/:${PROJECT_SOURCE_DIR}/stbox/src/:${PROJECT_SOURCE_DIR}/stbox/src/tsgx/crypto/
  )

target_link_libraries(us_covid19_parser stbox_common_t stbox_channel_t ypc_analyzer_t)


enclave_sign(us_covid19_parser KEY enclave_private.pem
  CONFIG enclave.config.xml)
