
add_definitions(-DHPDA_DEBUG)

add_library(hpda src/extractor/internal/https_request.cpp
  src/extractor/paged_https_extractor.cpp
  src/engine/engine.cpp
  src/engine/functor.cpp)
target_link_libraries(hpda pthread ssl crypto boost_system curl)
install(TARGETS hpda DESTINATION lib)

if(SGX_MODE STREQUAL "Debug")
  add_subdirectory(test)
  #add_subdirectory(example)
endif()
