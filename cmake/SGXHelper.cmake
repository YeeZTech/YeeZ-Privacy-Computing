
include(FidEdlFingerprint)

function(add_ypc_applet target)
  set(oneValueArgs CRYPTO)
  set(multiValueArgs SRCS)
  cmake_parse_arguments(ADD_YPC_APPLET "${options}"
      "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if((NOT ADD_YPC_APPLET_CRYPTO STREQUAL "stdeth") AND (NOT APP_YPC_APPLET_CRYPTO STREQUAL "gmssl"))
      message(FATAL_ERROR "add_ypc_applet can only use 'stdeth' or 'gmssl' for CRYPTO " )
    endif()

  fid_compute_edl_fingerprint(
    applet_edl_fingerprint
    "${PROJECT_SOURCE_DIR}/include/ypc/edl/eparser.edl"
    "${PROJECT_SOURCE_DIR}/include/"
    "${PROJECT_SOURCE_DIR}/include/ypc/edl/util/"
    "${PROJECT_SOURCE_DIR}/include/ypc/edl/"
    "${PROJECT_SOURCE_DIR}/include/ypc/stbox/")
  add_enclave_library(${target} SRCS ${ADD_YPC_APPLET_SRCS}
    EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/eparser.edl
    EDL_SEARCH_PATHS "${PROJECT_SOURCE_DIR}/include/:${PROJECT_SOURCE_DIR}/include/ypc/edl/util/:${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox/"
    EDL_DEPENDS ${applet_edl_fingerprint_DEPENDENCIES}
  )
  # Parser-plugin packaging is a toolkit concern.  Tag the applet only when
  # the analyzer supplied its registry module.
  if(COMMAND fid_mark_parser_enclave)
    fid_mark_parser_enclave(${target}
      EDL "${PROJECT_SOURCE_DIR}/include/ypc/edl/eparser.edl"
      EDL_SEARCH_PATHS
        "${PROJECT_SOURCE_DIR}/include/"
        "${PROJECT_SOURCE_DIR}/include/ypc/edl/util/"
        "${PROJECT_SOURCE_DIR}/include/ypc/edl/"
        "${PROJECT_SOURCE_DIR}/include/ypc/stbox/")
  endif()

  target_include_directories(${target}-edlobj PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

  target_include_directories(${target} PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )
  target_link_libraries(${target} PRIVATE
    stbox_common_t
    stbox_channel_t
    analyzer_t ${ADD_YPC_APPLET_CRYPTO}_t)
endfunction()

