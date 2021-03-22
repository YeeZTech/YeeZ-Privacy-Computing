#include "db.h"
#include "net.h"
#include "ypc/configuration.h"
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/program_options.hpp>
#include <errno.h>
#include <fcntl.h>
#include <glog/logging.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

boost::program_options::variables_map parse_command_line(int argc,
                                                         const char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("Yeez Privacy Computing Daemon options");
  bp::options_description general("General options");
  bp::options_description status("Status options");
  bp::options_description other("Options");

  // clang-format off
  general.add_options()
    ("help", "help message");

  status.add_options()
    ("status", "show status");

  other.add_options()
    ("init", "initialize db")
    ("start", "start the daemon")
    ("stop", "stop the daemon")
    ("clear-db", "clear all data in db")
    ("auto-fix", "try fix some problems");
  // clang-format on

  all.add(general).add(status).add(other);
  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, all), vm);

  if (vm.count("help")) {
    std::cout << all << std::endl;
    exit(-1);
  }
  return vm;
}

namespace toolkit {
namespace ypcd {
extern int daemon_main(const std::string &conf_file,
                       ::ff::sql::mysql<::ff::sql::cppconn> *db);
}
} // namespace toolkit

void release_lock() {
  boost::interprocess::named_mutex mutex(boost::interprocess::open_or_create,
                                         "tech.yeez.ypcd");
  mutex.unlock();
}

int main(int argc, const char *argv[]) {
  boost::program_options::variables_map vm;
  try {
    vm = parse_command_line(argc, argv);
  } catch (...) {
    std::cout << "invalid cmd line parameters!" << std::endl;
    return -1;
  }

  auto conf_file_name = "ypcd.conf";
  ypc::db_info_t dbinfo;
  try {
    dbinfo = ypc::configuration::instance().read_db_config_file(conf_file_name);
  } catch (std::exception &e) {
    std::cout << "Got error: " << e.what() << std::endl;
    return -1;
  }

  auto ddb_ptr = std::make_shared<toolkit::ypcd::ypcd_db>(
      dbinfo.get<db_url>(), dbinfo.get<db_usr>(), dbinfo.get<db_pass>(),
      dbinfo.get<db_dbname>());
  if (vm.count("clear-db")) {
    try {
      ddb_ptr->clear_tables();
    } catch (...) {
      std::cout << "seems nothing to clear" << std::endl;
    }
    std::cout << "trying to create tables...";
    ddb_ptr->create_tables();
    std::cout << " done" << std::endl;
    return 0;
  }
  if (vm.count("auto-fix")) {
    release_lock();
    return 0;
  }
  auto *db = ddb_ptr->db_engine_ptr();
  if (vm.count("init")) {
    db->create_database();
    std::cout << " done " << std::endl;
    return 0;
  }

  if (vm.count("status")) {
    std::cout << "TODO: show status here" << std::endl;
    return 0;
  }
  if (vm.count("start")) {
    if (!db->is_ready()) {
      std::cout << "DB not ready. Please run with --init first." << std::endl;
      return 0;
    }
    /* Our process ID and Session ID */
    pid_t pid, sid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
      exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then we can exit the parent process. */
    if (pid > 0) {
      exit(EXIT_SUCCESS);
    }

    boost::interprocess::named_mutex mutex(boost::interprocess::open_or_create,
                                           "tech.yeez.ypcd");
    if (!mutex.try_lock()) {
      std::cout << "Daemon process already exists!" << std::endl;
      std::cout << " If there is no daemon process running, you may try "
                   "--auto-fix. "
                << std::endl;
      return 0;
    }
    umask(0);

    std::string log_dir =
        ypc::configuration::instance().create_logdir_if_not_exist();
    FLAGS_log_dir = log_dir.c_str();
    google::InitGoogleLogging(argv[0]);

    sid = setsid();
    if (sid < 0) {
      LOG(ERROR) << "sid failure";
      return EXIT_FAILURE;
    }

    if ((chdir("/")) < 0) {
      LOG(ERROR) << "chdir failed";
      return EXIT_FAILURE;
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    LOG(INFO) << "close std file desc";
    toolkit::ypcd::daemon_main(conf_file_name, db);
    mutex.unlock();
    return 0;
  }

  if (vm.count("stop")) {
    toolkit::ypcd::send_exit_code(conf_file_name);
    release_lock();
    return 0;
  }
  std::cout << "Nothing to do." << std::endl;
  return 0;
}
