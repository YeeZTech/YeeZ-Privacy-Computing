add_fid_parser_plugin(
  TARGET fidelius_eparser_v1_plugin
  MODULE_ID fidelius.eparser.v1
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/eparser.edl
  EDL_SEARCH_PATHS
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/include/ypc/edl/
    ${PROJECT_SOURCE_DIR}/include/ypc/stbox/
    ${PROJECT_SOURCE_DIR}/include/ypc/edl/util/
  SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/eparser_plugin.cpp)

add_subdirectory(dianshu)
