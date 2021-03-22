#include "net.h"
#include "common/package.h"
#include "services.h"
#include "ypc/configuration.h"
#include <glog/logging.h>
#include <iostream>

namespace toolkit {
namespace ypcd {

void start_net_service(const std::string &conf_file,
                       ::ff::sql::mysql<::ff::sql::cppconn> *db) {
  ::ypc::net_info_t ni =
      ::ypc::configuration::instance().read_net_info_from_file(conf_file);

  ::ff::net::net_nervure nn;
  ::ff::net::typed_pkg_hub hub;

  hub.to_recv_pkg<ctrl_pkg_t>([&](std::shared_ptr<ctrl_pkg_t>) {
    LOG(INFO) << "Got ctrl_pkg_t, to exit...";
    nn.stop();
  });
  hub.tcp_to_recv_pkg<register_data_meta_pkg_t>(
      [&](std::shared_ptr<register_data_meta_pkg_t> pkg,
          ::ff::net::tcp_connection_base *from) {
        LOG(INFO) << "Got register data meta pkg";
        // TODO
        toolkit::ypcd::register_data_meta_service(pkg.get(), db);

        auto ack_pkg = std::make_shared<ack_pkg_t>();
        ack_pkg->set<ack_type_c>(register_data_meta_pkg_id);
        from->send(ack_pkg);
      });

  nn.add_pkg_hub(hub);
  nn.add_tcp_server("127.0.0.1", ni.get<ctrl_net_port>());

  LOG(INFO) << "start network server";
  nn.run();
}

void send_exit_code(const std::string &conf_file) {
  ::ypc::net_info_t ni =
      ::ypc::configuration::instance().read_net_info_from_file(conf_file);

  ::ff::net::net_nervure nn;
  std::atomic<int> got_disconnect(0);
  std::atomic<int> connected(0);
  std::thread checker_thrd([&]() {
    std::cout << "Checking the daemon process..." << std::endl;
    for (int i = 0; i < 10; ++i) {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(1s);
      if (got_disconnect) {
        break;
      }
    }
    if (connected) {
      if (!got_disconnect) {
        std::cout << " Seems the daemon process is busy, no response. You may "
                     "try later."
                  << std::endl;
      }
    } else {
      std::cout << "Not got any daemon process response" << std::endl;
    }
  });

  nn.get_event_handler()->listen<::ff::net::event::tcp_get_connection>(
      [&](::ff::net::tcp_connection_base *conn) {
        connected = 1;
        auto pkg = std::make_shared<ctrl_pkg_t>();
        conn->send(pkg);
        std::cout << "Waiting for exit..." << std::endl;
      });
  nn.get_event_handler()->listen<::ff::net::event::tcp_lost_connection>(
      [&](::ff::net::tcp_connection_base *) {
        std::cout << " Done" << std::endl;
        nn.stop();
        got_disconnect = 1;
      });

  nn.add_tcp_client("127.0.0.1", ni.get<ctrl_net_port>());

  nn.run();

  checker_thrd.join();
}
} // namespace ypcd
} // namespace toolkit
