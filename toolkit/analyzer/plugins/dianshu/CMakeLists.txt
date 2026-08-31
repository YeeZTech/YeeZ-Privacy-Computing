find_package(OpenCV REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(LibArchive REQUIRED)

set(dianshu_plugin_sources
  dianshu_parser_plugin.cpp
  handler/mp4_handler.cpp
  handler/csv_and_tsv_handler.cpp
  handler/pdf_handler.cpp
  handler/zip_and_rar_handler.cpp
  handler/mp4_cut_handler.cpp)

# This DSO owns the complete untrusted side of dianshu_parser.edl, including
# its heavy media/document handlers.  fid_analyzer sees only the stable C ABI.
add_fid_parser_plugin(
  TARGET fid_dianshu_eparser_plugin
  MODULE_ID dianshu.eparser.v1
  EDL ${CMAKE_CURRENT_SOURCE_DIR}/dianshu_parser.edl
  EDL_SEARCH_PATHS
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/include/ypc/edl/util/
    ${PROJECT_SOURCE_DIR}/include/ypc/edl/
    ${PROJECT_SOURCE_DIR}/include/ypc/stbox/
  SRCS ${dianshu_plugin_sources}
  LINK_LIBRARIES
    stbox_common
    core
    glog
    boost_filesystem
    boost_system
    ${OpenCV_LIBS}
    ${LibArchive_LIBRARIES}
    zip
    poppler-cpp
    Qt5::Widgets)

target_include_directories(fid_dianshu_eparser_plugin PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/handler
  ${PROJECT_SOURCE_DIR}/include
  ${FF_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${LibArchive_INCLUDE_DIRS})

# The copied contract is installed with the offline registry utility so
# already-published signed enclaves can be registered by hash/MRENCLAVE without
# modifying or re-signing them.
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dianshu_parser.edl
  DESTINATION share/fidelius/registry-tools)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/RegisterPublishedDianshuEnclaves.cmake
  DESTINATION share/fidelius/registry-tools)
