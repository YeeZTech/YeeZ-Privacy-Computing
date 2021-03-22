add_library(person_reader
  SHARED
  person_reader.cpp)

target_link_libraries(person_reader ypc_core)
