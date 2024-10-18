set(iris_srcs iris_parser_module.cpp)
add_untrusted_library(iris_parser_module SHARED
  SRCS ${iris_srcs}
  EDL ${PROJECT_SOURCE_DIR}/example/iris/analyzer/enclave/iris.edl
  EDL_SEARCH_PATHS "${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox/:${PROJECT_SOURCE_DIR}/include/ypc/edl/util")

target_link_libraries(iris_parser_module PUBLIC stbox_common)
#install(TARGETS ypc_parser_module DESTINATION lib COMPONENT c_ypc_core OPTIONAL)

target_include_directories(iris_parser_module PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_include_directories(iris_parser_module INTERFACE
  "$<INSTALL_INTERFACE:${SGX_INCLUDE_DIR}>")
target_link_directories(iris_parser_module INTERFACE
  "$<INSTALL_INTERFACE:${SGX_LIBRARY_DIR}>")

set(personlist_srcs personlist_parser_module.cpp)
add_untrusted_library(personlist_parser_module SHARED
  SRCS ${personlist_srcs}
  EDL ${PROJECT_SOURCE_DIR}/example/personlist/first_match/enclave/personlist.edl
  EDL_SEARCH_PATHS "${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox/:${PROJECT_SOURCE_DIR}/include/ypc/edl/util")

target_link_libraries(personlist_parser_module PUBLIC stbox_common)
#install(TARGETS ypc_parser_module DESTINATION lib COMPONENT c_ypc_core OPTIONAL)

target_include_directories(personlist_parser_module PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_include_directories(personlist_parser_module INTERFACE
  "$<INSTALL_INTERFACE:${SGX_INCLUDE_DIR}>")
target_link_directories(personlist_parser_module INTERFACE
  "$<INSTALL_INTERFACE:${SGX_LIBRARY_DIR}>")

