
add_untrusted_library(ypc_datahub_module SHARED
  SRCS "datahub_sgx_module.cpp"
  EDL ${PROJECT_SOURCE_DIR}/toolkit/datahub/enclave/enclave.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/core/edl/:${PROJECT_SOURCE_DIR}/stbox/src/)

add_untrusted_library(ypc_parser_module SHARED
  SRCS "parser_sgx_module.cpp"
  EDL ${PROJECT_SOURCE_DIR}/core/edl/eparser.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/core/edl/:${PROJECT_SOURCE_DIR}/stbox/src/:${PROJECT_SOURCE_DIR}/stbox/src/tsgx/crypto/)

target_link_libraries(ypc_datahub_module stbox_common_u)
target_link_libraries(ypc_parser_module stbox_common_u)
