add_library(libypcd
  db.cpp
  daemon_main.cpp
  services.cpp
  net.cpp)
target_link_libraries(libypcd mysqlcppconn ypc_core glog pthread ff_net )

add_executable(ypcd
  main.cpp)
target_link_libraries(ypcd libypcd)

