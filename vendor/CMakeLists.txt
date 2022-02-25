cmake_minimum_required(VERSION 2.8)
project(fflib)

add_definitions(-std=c++11)
add_definitions(-O2)
#add_definitions(-g)
#add_definitions(-DPROTO_BUF_SUPPORT)


include_directories(/usr/local/include)
include_directories(${PROJECT_SOURCE_DIR}/include)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
link_directories(/usr/local/lib)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)


file(GLOB_RECURSE functionflow_src src/functionflow/*.cpp)
file(GLOB_RECURSE net_src src/net/*.cpp)

set(lib_list glog gflags)

find_package(Protobuf QUIET)
if(PROTOBUF_FOUND)
  #message(STATUS "found protobuf, and include protobuf support")
  #add_definitions(-DPROTO_BUF_SUPPORT)
  #include_directories(${PROTOBUF_INCLUDE_DIRS})
  #set(src_list ${src_list} extension/protobuf/proto_buf_wrapper_pkg.cpp
  #  extension/protobuf/protobuf_pkg_hub.cpp)
  #set(lib_list ${lib_list} ${PROTOBUF_LIBRARIES})
else()
  message(STATUS "not found protobuf, gonna ignore protobuf support")
endif()

add_library(ff_functionflow EXCLUDE_FROM_ALL  SHARED ${functionflow_src})
#add_library(functionflow_static EXCLUDE_FROM_ALL STATIC ${functionflow_src})
#set_target_properties(functionflow_dynamic PROPERTIES OUTPUT_NAME "ff_functionflow")
#set_target_properties(functionflow_static  PROPERTIES OUTPUT_NAME "ff_functionflow")
install(TARGETS ff_functionflow DESTINATION lib COMPONENT fflib OPTIONAL)

add_library(ff_net EXCLUDE_FROM_ALL SHARED ${net_src})
#add_library(net_static EXCLUDE_FROM_ALL STATIC ${net_src})
#target_link_libraries(net_dynamic boost_program_options boost_system)
#target_link_libraries(net_static boost_program_options boost_system)
#set_target_properties(net_dynamic PROPERTIES OUTPUT_NAME "ff_net")
#set_target_properties(net_static  PROPERTIES OUTPUT_NAME "ff_net")
install(TARGETS ff_net DESTINATION lib COMPONENT fflib OPTIONAL)
if(Release)
else()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/)
link_directories(${PROJECT_SOURCE_DIR}/lib)
add_subdirectory(example)
endif()
