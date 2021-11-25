#set(TESTING_CONFIG_OPT FILEPATH ${CMAKE_CURRENT_LIST_DIR}/config.cmake)
#include("cmake/HunterGate.cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#HunterGate(
  #URL "https://github.com/cpp-pm/hunter/archive/v0.23.231.tar.gz"
  #SHA1 "3f9910d70a5be7005a2f23448aff4f7de5ce6bed"
#)

project(YeezPrivacyComputingSDK)
cmake_minimum_required(VERSION 3.9.3)
add_definitions(-std=c++14)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
find_package(SGX REQUIRED)

enable_testing()

if(SGX_MODE STREQUAL "Debug")
	message(STATUS "This is a Debug version!")
	add_definitions(-g)
  add_definitions(-O0)
  configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
      message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
    if(result)
      message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    # Prevent overriding the parent project's compiler/linker
    # settings on Windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    # Add googletest directly to our build. This defines
    # the gtest and gtest_main targets.
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                    ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                    EXCLUDE_FROM_ALL)

    # The gtest/gtest_main targets carry header search path
    # dependencies automatically when using CMake 2.8.11 or
    # later. Otherwise we have to add them here ourselves.
    if (CMAKE_VERSION VERSION_LESS 2.8.11)
      include_directories("${gtest_SOURCE_DIR}/include")
    endif()
else()
  message(STATUS "This is a " ${SGX_MODE} " version!")
	add_definitions(-O2)
	add_definitions(-DNDEBUG)
endif()


include_directories(${SGX_INCLUDE_DIR})
include_directories(/usr/local/include)
link_directories(/usr/local/lib)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/hpda/include)
include_directories(${PROJECT_SOURCE_DIR}/core/include)
include_directories(${PROJECT_SOURCE_DIR}/stbox/include)
include_directories(${PROJECT_SOURCE_DIR}/tookit/include)
include_directories(${PROJECT_SOURCE_DIR}/vendor/fflib/include/)
link_directories(${PROJECT_SOURCE_DIR}/vendor/fflib/lib/)
link_directories(${SGX_LIBRARY_DIR})

#hunter_add_package(Boost)
#find_package(Boost CONFIG REQUIRED)

set(FF_PATH ${PROJECT_SOURCE_DIR}/vendor/fflib)
set(FF_SRC_PATH ${FF_PATH}/src)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib/)

add_subdirectory(vendor/pybind11)

add_subdirectory(common)
add_subdirectory(hpda)
add_subdirectory(core)
add_subdirectory(toolkit)
add_subdirectory(stbox)

if(SGX_MODE STREQUAL "Debug")
  add_subdirectory(test)
endif()
add_subdirectory(keymgr)
add_subdirectory(example)

