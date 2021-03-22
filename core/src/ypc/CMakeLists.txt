add_subdirectory(sgx)

add_library(ypc_core SHARED
  base64.cpp
  base58.cpp
  byte.cpp
  timer_loop.cpp
  sha.cpp
  db.cpp
  configuration.cpp
  sealed_file.cpp
  command_executor.cpp
  filesystem.cpp
  privacy_data_reader.cpp
  to_hex.cpp
  ref.cpp
  exceptions.cpp
  ${PROJECT_SOURCE_DIR}/vendor/fflib/src/net/common/archive.cpp)
  #${PROJECT_SOURCE_DIR}/vendor/fflib/src/net/middleware/package.cpp)

target_link_libraries(ypc_core mysqlcppconn cryptopp boost_program_options boost_filesystem boost_system glog dl)

