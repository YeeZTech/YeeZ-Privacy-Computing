set(srcs iris_parser_module.cpp)
add_untrusted_library(iris_parser_module SHARED
  SRCS ${srcs}
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

