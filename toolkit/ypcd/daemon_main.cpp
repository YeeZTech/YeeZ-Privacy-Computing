#include "net.h"
#include "services.h"
#include <chrono>
#include <fstream>
#include <glog/logging.h>
#include <thread>

namespace toolkit {
namespace ypcd {

int daemon_main(const std::string &conf_file,
                ::ff::sql::mysql<::ff::sql::cppconn> *db) {
  std::thread net_thread([&]() {
    auto engine_copy = db->thread_copy();
    start_net_service(conf_file, engine_copy.get());
  });
  // Add other service here
  std::thread data_thread([&]() { start_data_analysis_service(); });
  data_thread.join();
  net_thread.join();
}
} // namespace ypcd
} // namespace toolkit
