#include "net.h"
#include "common/package.h"
#include "ypc/configuration.h"
#include <glog/logging.h>

namespace toolkit {
namespace shuttle {

void send_data_meta(const std::string &data_id, const std::string &data_name,
                    const std::string &exec_parser_path,
                    const std::string &exec_parser_param,
                    const std::string &sealed_data_path) {
  ::ypc::net_info_t ni =
      ::ypc::configuration::instance().read_net_info_from_file("ypcd.conf");

  ::ff::net::net_nervure nn;

  ::ff::net::typed_pkg_hub hub;
  std::atomic<int> got_disconnect(0);
  std::atomic<int> connected(0);

  hub.tcp_to_recv_pkg<ack_pkg_t>([&](std::shared_ptr<ack_pkg_t> pkg,
                                     ::ff::net::tcp_connection_base *from) {
    LOG(INFO) << "Got confirm, to exit...";
    got_disconnect = 1;
    nn.stop();
  });
  nn.add_pkg_hub(hub);

  std::thread checker_thrd([&]() {
    std::cout << "Checking the YPCD process..." << std::endl;
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
        auto pkg = std::make_shared<register_data_meta_pkg_t>();
        pkg->set<data_type_c, data_id_c, data_desc_c, sealed_data_path_c>(
            dt_file, data_id, data_name, sealed_data_path);
        pkg->set<exec_parser_path_c, exec_parser_param_c>(exec_parser_path,
                                                          exec_parser_param);
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

  try {
    nn.run();
  } catch (std::exception &e) {
    if (!got_disconnect) {
      LOG(INFO) << "Got exception " << e.what();
    }
  }

  checker_thrd.join();
}
} // namespace shuttle
} // namespace toolkit
