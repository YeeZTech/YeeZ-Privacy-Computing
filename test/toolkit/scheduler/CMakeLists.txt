#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add_executable(test_scheduler
#    test_scheduler.cpp)
## external libraries
#target_include_directories(test_scheduler PRIVATE
#    "${PROJECT_SOURCE_DIR}/vendor/json/include")
## internal headers
#target_include_directories(test_scheduler PUBLIC
#    "${PROJECT_SOURCE_DIR}/toolkit/analyzer"
#    "${PROJECT_SOURCE_DIR}/toolkit/datahub"
#    "${PROJECT_SOURCE_DIR}/toolkit/ydump")
## internal links
#target_link_directories(test_scheduler PUBLIC
#    "${PROJECT_SOURCE_DIR}/lib")
#target_link_libraries(test_scheduler
#    debug scheduler_debug optimized scheduler
#    debug fid_analyzer_debug optimized fid_analyzer
#    debug data_provider_debug optimized data_provider
#    debug ydump_debug optimized ydump)
