#set(TESTING_CONFIG_OPT FILEPATH ${CMAKE_CURRENT_LIST_DIR}/config.cmake)
#include("cmake/HunterGate.cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.12)
project(YPC VERSION 0.5.2)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if(CMAKE_BUILD_TYPE STREQUAL "")
  message(STATUS "CMAKE_BUILD_TYPE not defined, use Debug as default")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(SGX_MODE "Debug")
  set(SGX_HW OFF)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set(SGX_MODE "PreRelease")
  set(SGX_HW ON)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(SGX_MODE "Release")
  set(SGX_HW ON)
endif()


if(SGX_MODE STREQUAL "Debug" AND SGX_HW)
  set(SGX_HW OFF)
  message(STATUS "Cannot use SGX_HW=ON when SGX_MODE is Debug, correct it to OFF")
endif()

if(SGX_MODE STREQUAL "Release" AND NOT SGX_HW)
  set(SGX_HW ON)
  message(STATUS "Cannot use SGX_HW=OFF when SGX_MODE is Release, correct it to ON")
endif()

include(vendor)
include(Version)
include(Coverage)
include(Dart)
include(Doxygen)

find_package(SGX)
if(SGX_FOUND)
  message(STATUS "found SGX, build full support")
  add_definitions(-DYPC_ENABLE_SGX)
else()
  message(STATUS "SGX not found, ignore support for SGX")
endif()

include(SGXHelper)

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

set(CPACK_PACKAGE_NAME "ypc")
set(CPACK_PACKAGE_VENDOR "YeeZTech")
set(CPACK_PACKAGE_CONTACT "fidelius@yeez.tech")
set(CPACK_PACKAGE_DESCRIPTION "Fidelius: YeeZ Privacy Computing")
set(CPACK_COMPONENTS_GROUPING ONE_PER_GROUP)

include(CPack)

configure_file("${PROJECT_SOURCE_DIR}/cmake/version.h.in" "${PROJECT_SOURCE_DIR}/include/ypc/version.h")
configure_file("${PROJECT_SOURCE_DIR}/cmake/project_path.h.in" "${PROJECT_SOURCE_DIR}/test/project_path.h")

set(FF_PATH ${PROJECT_SOURCE_DIR}/vendor/fflib)
set(FF_SRC_PATH ${FF_PATH}/src)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/)

include(ClangTidy)
include(GNUInstallDirs)
set(config_install_dir "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(include_install_dir "${CMAKE_INSTALL_PREFIX}/include")
set(lib_install_dir "${CMAKE_INSTALL_PREFIX}/lib")
set(bin_install_dir "${CMAKE_INSTALL_PREFIX}/bin")

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

install(DIRECTORY include/hpda include/toolkit include/ypc
  DESTINATION "${include_install_dir}")

message(STATUS "--------SGX_MODE: " ${SGX_MODE})
message(STATUS "--------SGX_HW: " ${SGX_HW})
add_subdirectory(vendor/pybind11)

add_subdirectory(ypc)
add_subdirectory(hpda)
if(SGX_FOUND)
add_subdirectory(example)
endif()

if(SGX_MODE STREQUAL "Debug")
  include(GTest)
  enable_testing()
  add_subdirectory(test)
endif()

Doxygen(${PROJECT_SOURCE_DIR})

add_subdirectory(toolkit)
add_subdirectory(test/integrate)

install(
  FILES "${project_config}" "${version_config}" "${util_config}"
  DESTINATION "${config_install_dir}"
  COMPONENT cmake
)
install(
  FILES "${PROJECT_SOURCE_DIR}/cmake/FindSGX.cmake"
  DESTINATION "${config_install_dir}"
  COMPONENT sgxcmake
)


if (UNIX)
  #install(CODE "execute_process(COMMAND ldconfig)")
endif()

