#set(TESTING_CONFIG_OPT FILEPATH ${CMAKE_CURRENT_LIST_DIR}/config.cmake)
#include("cmake/HunterGate.cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.12)
project(YPC VERSION 0.2.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if(NOT DEFINED SGX_HW)
if(SGX_MODE STREQUAL "Debug")
  set(SGX_HW OFF)
else()
  set(SGX_HW ON)
endif()
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE)
if(SGX_MODE STREQUAL "Debug")
  set(CMAKE_BUILD_TYPE "Debug")
elseif(SGX_MODE STREQUAL "PreRelease")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
else()
  set(CMAKE_BUILD_TYPE "Release")
endif()
endif()

include(vendor)
include(Version)

find_package(SGX)
if(SGX_FOUND)
  message(STATUS "found SGX, build full support")
else()
  message(STATUS "SGX not found, ignore support for SGX")
endif()

include(GTest)
include(SGXHelper)

enable_testing()

if(SGX_MODE STREQUAL "Debug")
  message(STATUS "This is a Debug version!")
  add_definitions(-DDEBUG)
  add_definitions(-g)
  add_definitions(-O0)
else()
  message(STATUS "This is a " ${SGX_MODE} " version!")
  add_definitions(-O2)
  add_definitions(-DNDEBUG)
endif()

configure_file("${PROJECT_SOURCE_DIR}/cmake/version.h.in" "${PROJECT_SOURCE_DIR}/include/ypc/version.h")
configure_file("${PROJECT_SOURCE_DIR}/cmake/project_path.h.in" "${PROJECT_SOURCE_DIR}/test/project_path.h")

set(FF_PATH ${PROJECT_SOURCE_DIR}/vendor/fflib)
set(FF_SRC_PATH ${FF_PATH}/src)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/)

include(GNUInstallDirs)
set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(util_config "${generated_dir}/Util.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${version_config}" COMPATIBILITY SameMinorVersion)

set(LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})
configure_package_config_file("cmake/Util.cmake.in"
  "${util_config}"
  INSTALL_DESTINATION "${config_install_dir}"
  PATH_VARS PROJECT_NAME)

configure_package_config_file("cmake/Config.cmake.in"
  "${project_config}"
  INSTALL_DESTINATION "${config_install_dir}"
  PATH_VARS LIB_INSTALL_DIR INCLUDE_INSTALL_DIR PROJECT_NAME)


install(DIRECTORY include/
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/"
  MESSAGE_NEVER)

add_subdirectory(vendor/pybind11)

add_subdirectory(ypc)
add_subdirectory(hpda)
add_subdirectory(example)
add_subdirectory(toolkit)

if(SGX_MODE STREQUAL "Debug")
  add_subdirectory(test)
endif()

install(
  FILES "${project_config}" "${version_config}" "${util_config}"
  DESTINATION "${config_install_dir}"
)
install(
  FILES "${PROJECT_SOURCE_DIR}/cmake/FindSGX.cmake"
  DESTINATION "${config_install_dir}"
  )

if (UNIX)
  #install(CODE "execute_process(COMMAND ldconfig)")
endif()
