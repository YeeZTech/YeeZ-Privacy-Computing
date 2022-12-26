set(srcs cxxfile.cpp
  kv.cpp)
add_trusted_library(core_util_t
  core_util_t
  SRCS ${srcs}
  EDL ${PROJECT_SOURCE_DIR}/include/ypc/edl/eparser.edl
  EDL_SEARCH_PATHS "${PROJECT_SOURCE_DIR}/include/ypc/edl/:${PROJECT_SOURCE_DIR}/include/ypc/stbox/:${PROJECT_SOURCE_DIR}/include/ypc/edl/util/")

target_include_directories(core_util_t-edlobj PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  #"$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ypc/stbox>")

target_include_directories(core_util_t PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/ypc/stbox>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

install(TARGETS core_util_t EXPORT mod_core_util_t
  DESTINATION lib)

install(EXPORT mod_core_util_t
  DESTINATION "${config_install_dir}/core"
  NAMESPACE "${namespace}")

AddClangTidy(core_util_t)
