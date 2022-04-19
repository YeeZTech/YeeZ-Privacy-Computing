add_subdirectory(usgx)

add_untrusted_library(ypc_parser_module SHARED
  SRCS "parser_sgx_module.cpp"
  EDL ${PROJECT_SOURCE_DIR}/core/edl/eparser.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/core/edl/:${PROJECT_SOURCE_DIR}/stbox/src/:${PROJECT_SOURCE_DIR}/stbox/src/tsgx/crypto/)

if (${ENABLE_DIANSHU})
  set(ypc_usgx dshttp_request)
else()
  set(ypc_usgx ypc_usgx)
endif()
target_link_libraries(ypc_parser_module stbox_common_u ${ypc_usgx})
install(TARGETS ypc_parser_module DESTINATION lib COMPONENT c_ypc_core OPTIONAL)
