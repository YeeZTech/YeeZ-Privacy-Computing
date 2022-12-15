
function(add_ypc_applet target)
  set(oneValueArgs CRYPTO)
  set(multiValueArgs SRCS)
  cmake_parse_arguments(ADD_YPC_APPLET "${options}"
      "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if((NOT ADD_YPC_APPLET_CRYPTO STREQUAL "stdeth") AND (NOT APP_YPC_APPLET_CRYPTO STREQUAL "gmssl"))
      message(FATAL_ERROR "add_ypc_applet can only use 'stdeth' or 'gmssl' for CRYPTO " )
    endif()

  add_enclave_library(${target} SRCS ${ADD_YPC_APPLET_SRCS}
    EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/eparser.edl
    EDL_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox/
  )
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

