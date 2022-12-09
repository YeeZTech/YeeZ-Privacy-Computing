find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/vendor/fflib/CMakeLists.txt")
    message(FATAL_ERROR "The submodule fflib were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/vendor/fflib/CMakeLists.txt")
  #add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/fflib/")
  if(NOT EXISTS ${PROJECT_SOURCE_DIR}/vendor/fflib/lib/)
    file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/vendor/fflib/build/)
    execute_process(COMMAND ${CMAKE_COMMAND} -B ./build
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/vendor/fflib/)

    execute_process(COMMAND ${CMAKE_COMMAND} --build ./build
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/vendor/fflib/)
  endif()
endif()

set(FF_PATH "${PROJECT_SOURCE_DIR}/vendor/fflib")
set(FF_LIB_DIR "${FF_PATH}/lib")
set(FF_INCLUDE_DIR "${FF_PATH}/include")
