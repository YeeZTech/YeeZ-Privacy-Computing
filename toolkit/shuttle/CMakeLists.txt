add_library(libypcshuttle
  config.cpp
  net.cpp
  )
target_link_libraries(libypcshuttle ypc_core glog pthread ff_net)

add_executable(ypcshuttle
  main.cpp)
target_link_libraries(ypcshuttle libypcshuttle)

