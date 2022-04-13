set(src parser_wrapper_base.cpp
  extra_data_usage.cpp
  extra_data_source.cpp
  ../ecommon/package.cpp
  ../ecommon/signer_verify.cpp
  ${FF_SRC_PATH}/net/common/archive.cpp
  ${PROJECT_SOURCE_DIR}/hpda/src/engine/engine.cpp
  ${PROJECT_SOURCE_DIR}/hpda/src/engine/functor.cpp
  #${FF_SRC_PATH}/net/middleware/package.cpp
  )

add_trusted_library(ypc_analyzer_t
  ypc_analyzer
  SRCS ${src}
  EDL ${PROJECT_SOURCE_DIR}/core/edl/yaenclave.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/core/edl)
target_link_libraries(ypc_analyzer_t stbox_channel_t stbox_crypto_t)
#install(TARGETS ypc_analyzer_t DESTINATION lib COMPONENT c_ypc_core OPTIONAL)
