add_library(iris_reader
  SHARED
  iris_reader.cpp)
install(TARGETS iris_reader DESTINATION lib)
