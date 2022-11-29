

add_executable(personlist_gen_multi main.cpp)

target_link_libraries(personlist_gen_multi ff_net core)
target_link_directories(personlist_gen_multi PUBLIC ${FF_LIB_DIR})
