add_library(train_a_reader SHARED train_a_reader.cpp)
target_link_libraries(train_a_reader pthread ff_net)

add_library(train_b_reader SHARED train_b_reader.cpp)
target_link_libraries(train_b_reader pthread ff_net)
