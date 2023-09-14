set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(scheduler
    main.cpp
    scheduler.cpp)
target_include_directories(scheduler PRIVATE
    "${PROJECT_SOURCE_DIR}/vendor/json/include"
    "${PROJECT_SOURCE_DIR}/vendor/taskflow")
install(TARGETS scheduler
  DESTINATION "${bin_install_dir}")

add_library(scheduler_lib
    lib.cpp
    scheduler.cpp
    dag.cpp)
set_property(TARGET scheduler_lib PROPERTY OUTPUT_NAME scheduler)
target_include_directories(scheduler_lib PRIVATE
    "${PROJECT_SOURCE_DIR}/vendor/json/include"
    "${PROJECT_SOURCE_DIR}/vendor/taskflow")
install(TARGETS scheduler_lib
    DESTINATION "${bin_install_dir}")
