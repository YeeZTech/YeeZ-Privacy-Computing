include_directories(${PROJECT_SOURCE_DIR}/include/ypc/stbox)
add_executable(personlist_gen main.cpp)

target_link_libraries(personlist_gen ff_net core)
target_link_directories(personlist_gen PUBLIC ${FF_LIB_DIR})
