add_subdirectory(enclave)
add_executable(iris_gen_model main.cpp)
target_link_libraries(iris_gen_model core)

add_executable(iris_gen_classify_input input.cpp)
target_link_libraries(iris_gen_classify_input core boost_program_options)
target_include_directories(iris_gen_classify_input PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )
