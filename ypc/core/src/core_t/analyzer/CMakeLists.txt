set(ASM_FILE ${PROJECT_SOURCE_DIR}/ypc/core/src/core_t/analyzer/oblivious_functions.asm)

set(src
  analyzer_version.cpp
  internal/keymgr_session.cpp
  var/enclave_hash_var.cpp
  ../ecommon/package.cpp
  ../ecommon/signer_verify.cpp
  ${FF_PATH}/src/net/common/archive.cpp
  ${PROJECT_SOURCE_DIR}/hpda/src/engine/engine.cpp
  ${PROJECT_SOURCE_DIR}/hpda/src/engine/functor.cpp
  )



# TODO:oblivious_functions.o是需要作为enclave的可信库还是非可信库？
add_custom_target(asm_target ALL COMMAND nasm -f elf64 ${ASM_FILE} -o ${PROJECT_SOURCE_DIR}/lib/oblivious_functions.o)

# add_custom_target(
#     clean_oblivious_functions
#     COMMAND ${CMAKE_COMMAND} -E remove
#         ${PROJECT_SOURCE_DIR}/lib/oblivious_functions.o
# )
# add_dependencies(clean clean_oblivious_functions)



add_trusted_library(analyzer_t
  analyzer
  SRCS ${src}
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/yaenclave.edl
  EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/edl)

target_include_directories(analyzer_t-edlobj PRIVATE
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ypc/stbox>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  )
target_include_directories(analyzer_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ypc/stbox>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ypc/stbox>"
  )

target_link_libraries(analyzer_t PUBLIC stbox_channel_t stbox_crypto_t ${PROJECT_SOURCE_DIR}/lib/oblivious_functions.o)

install(TARGETS analyzer_t
  EXPORT mod_analyzer_t
  DESTINATION "${lib_install_dir}"
  COMPONENT core_t)

install(EXPORT mod_analyzer_t
  DESTINATION "${config_install_dir}/core"
  NAMESPACE "${namespace}"
  COMPONENT core_t)

AddClangTidy(analyzer_t)
