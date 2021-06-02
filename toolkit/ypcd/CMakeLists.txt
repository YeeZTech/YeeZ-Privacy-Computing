add_library(libypcd
  db.cpp
  daemon_main.cpp
  services.cpp
  net.cpp)
target_link_libraries(libypcd mysqlcppconn ypc_core glog pthread ff_net )
install(TARGETS libypcd DESTINATION lib)

add_executable(ypcd
  main.cpp)
install(TARGETS ypcd DESTINATION bin)
target_link_libraries(ypcd libypcd)

