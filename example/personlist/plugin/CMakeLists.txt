add_library(person_reader
  SHARED
  person_reader.cpp)
install(TARGETS person_reader DESTINATION lib)

target_link_libraries(person_reader ypc_core)
