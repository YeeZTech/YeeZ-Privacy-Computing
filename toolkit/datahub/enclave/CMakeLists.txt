
include_directories(${PROJECT_SOURCE_DIR}/example)
include_directories(${PROJECT_SOURCE_DIR}/toolkit/datahub)
set(E_SRCS datahub_enclave.cpp hosting_data.cpp
  ${FF_SRC_PATH}/net/common/archive.cpp
  ${PROJECT_SOURCE_DIR}/core/src/ypc_t/ecommon/signer_verify.cpp
  )

add_enclave_library(edatahub SRCS ${E_SRCS}
  EDL datahub_enclave.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/core/edl/:${PROJECT_SOURCE_DIR}/stbox/src/)
target_link_libraries(edatahub stbox_crypto_t stbox_common_t stbox_channel_t )
install(TARGETS edatahub DESTINATION lib COMPONENT datahub OPTIONAL)
enclave_sign(edatahub KEY enclave_private.pem
  CONFIG datahub_enclave.config.xml)
install(FILES ${PROJECT_SOURCE_DIR}/lib/edatahub.signed.so DESTINATION lib COMPONENT datahub OPTIONAL)
