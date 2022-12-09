add_executable(find_person main.cpp)
target_link_libraries(find_person core stbox_common ff_net hpda)
target_link_directories(find_person PUBLIC ${FF_LIB_DIR})
