add_library(iris_reader SHARED reader.cpp)
target_link_libraries(iris_reader pthread ff_net)
target_include_directories(iris_reader PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )
target_link_directories(iris_reader PUBLIC ${FF_LIB_DIR})
