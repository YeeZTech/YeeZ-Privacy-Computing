add_library(iris_reader SHARED reader.cpp)
target_link_libraries(iris_reader pthread ff_net)
install(TARGETS iris_reader DESTINATION lib COMPONENT c_example_iris OPTIONAL)
