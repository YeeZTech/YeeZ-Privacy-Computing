include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/test/)
include_directories(${FF_INCLUDE_DIR})
link_directories(${FF_LIB_DIR})

add_subdirectory(core)
add_subdirectory(crypto)
add_subdirectory(keymgr)
add_subdirectory(toolkit)
