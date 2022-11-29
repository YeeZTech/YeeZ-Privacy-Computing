
find_package (Threads REQUIRED)

add_library(hpda SHARED src/extractor/internal/https_request.cpp
  src/extractor/paged_https_extractor.cpp
  src/engine/engine.cpp
  src/engine/functor.cpp)
target_link_libraries(hpda ssl crypto boost_system curl
  ${CMAKE_THREAD_LIBS_INIT}
  )
target_compile_options(hpda PUBLIC $<$<CONFIG:Debug>:-DHPDA_DEBUG>)

target_include_directories(hpda PUBLIC
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${FF_INCLUDE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )
install(TARGETS hpda EXPORT mod_hpda DESTINATION lib)
install(EXPORT mod_hpda
  DESTINATION "${config_install_dir}/hpda"
  NAMESPACE "${namespace}")

add_version(hpda)

if(SGX_MODE STREQUAL "Debug")
  add_subdirectory(test)
  #add_subdirectory(example)
endif()
