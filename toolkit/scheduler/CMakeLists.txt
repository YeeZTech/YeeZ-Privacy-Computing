set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(scheduler
    main.cpp
    scheduler.cpp)
target_include_directories(scheduler PUBLIC
    "${PROJECT_SOURCE_DIR}/vendor/json/include"
    "${PROJECT_SOURCE_DIR}/vendor/taskflow")
target_link_libraries(scheduler
    pthread)
install(TARGETS scheduler
  DESTINATION "${bin_install_dir}")

add_library(scheduler_lib
    lib.cpp
    scheduler.cpp
    dag.cpp)
set_property(TARGET scheduler_lib PROPERTY OUTPUT_NAME scheduler)
#set_target_properties(scheduler_lib PROPERTIES
#        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
target_include_directories(scheduler_lib PUBLIC
    "${PROJECT_SOURCE_DIR}/vendor/json/include"
    "${PROJECT_SOURCE_DIR}/vendor/taskflow")
target_link_libraries(scheduler_lib
    pthread)
install(TARGETS scheduler_lib
    DESTINATION "${bin_install_dir}")
