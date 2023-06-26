

add_executable(personlist_gen_oram main.cpp)

target_link_libraries(personlist_gen_oram ff_net core)
target_link_directories(personlist_gen_oram PUBLIC ${FF_LIB_DIR})
