#include "./extra_data_source.h"
#include "parser.h"
#include "sgx_bridge.h"
#include "ypc/configuration.h"
#include "ypc/ntobject_file.h"
#include "ypc/sealed_file.h"
#include "ypc/sha.h"
#include <boost/program_options.hpp>
#include <exception>
#include <fstream>
#include <iostream>
#include <stbox/stx_status.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using stx_status = stbox::stx_status;
using namespace ypc;

void parallel_parse(std::shared_ptr<param_source> psource,
                    const std::string &result_output_file,
                    const std::string &sealer_enclave_file,
                    const std::string &parser_enclave_file,
                    const std::string &keymgr_enclave_file,
                    const std::string &sealed_file);

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("Yeez Privacy Data Hub options");

  // clang-format off
  all.add_options()
    ("help", "help message")
    ("sealed-data-url", bp::value<std::string>(), "Sealed Data URL")
    ("sealer-path", bp::value<std::string>(), "sealer enclave path")
    ("parser-path", bp::value<std::string>(), "parser enclave path")
    ("keymgr-path", bp::value<std::string>(), "keymgr enclave path")
    ("source-type", bp::value<std::string>(), "input and output source type")
    // params read from database
    ("db-conf", bp::value<std::string>(), "database configuration file")
    ("request-hash", bp::value<std::string>(), "request hash")
    // params read from json file
    ("param-path", bp::value<std::string>(), "forward param path")
    ("result-path", bp::value<std::string>(), "output result path")
    ("extra-data-source", bp::value<std::string>(), "JSON file path which include extra data source information");
  // clang-format on

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, all), vm);

  if (vm.count("help")) {
    std::cout << all << std::endl;
    exit(-1);
  }
  return vm;
}

int main(int argc, char *argv[]) {
  boost::program_options::variables_map vm;
  try {
    vm = parse_command_line(argc, argv);
  } catch (...) {
    std::cout << "invalid cmd line parameters!" << std::endl;
    return -1;
  }
  if (!vm.count("parser-path")) {
    std::cout << "parser not specified!" << std::endl;
    return -1;
  }
  if (!vm.count("keymgr-path")) {
    std::cout << "keymgr not specified!" << std::endl;
    return -1;
  }
  if (!vm.count("sealed-data-url")) {
    std::cout << "sealed data url not specified" << std::endl;
    return -1;
  }
  if (!vm.count("sealer-path")) {
    std::cout << "sealer not specified" << std::endl;
    return -1;
  }
  if (!vm.count("source-type")) {
    std::cout << "source type not specified" << std::endl;
    return -1;
  }

  std::string sealed_file = vm["sealed-data-url"].as<std::string>();
  std::string sealer_enclave_file = vm["sealer-path"].as<std::string>();
  std::string parser_enclave_file = vm["parser-path"].as<std::string>();
  std::string keymgr_enclave_file = vm["keymgr-path"].as<std::string>();
  std::string source_type = vm["source-type"].as<std::string>();

  std::shared_ptr<param_source> psource;
  std::shared_ptr<result_target> rtarget;

  if (source_type == "json") {
    psource =
        std::make_shared<param_from_json>(vm["param-path"].as<std::string>());
    rtarget =
        std::make_shared<result_to_json>(vm["result-path"].as<std::string>());
  } else if (source_type == "db") {
    auto info = ypc::configuration::instance().read_db_config_file(
        vm["db-conf"].as<std::string>());
    psource = std::make_shared<param_from_db>(
        info.get<db_url>(), info.get<db_usr>(), info.get<db_pass>(),
        info.get<db_dbname>(),
        ypc::hex_bytes(vm["request-hash"].as<std::string>()).as<ypc::bytes>());
    rtarget = std::make_shared<result_to_db>(
        info.get<db_url>(), info.get<db_usr>(), info.get<db_pass>(),
        info.get<db_dbname>(),
        ypc::hex_bytes(vm["request-hash"].as<std::string>()).as<ypc::bytes>());
  } else {
    std::cout << "not supported source type" << std::endl;
    return -1;
  }

  bool is_sealed_file = true;
  try {
    ypc::simple_sealed_file ssf(sealed_file, true);
    std::cout << "valid sealed block file, use sequential mode" << std::endl;
  } catch (ypc::invalid_blockfile &e) {
    std::cout << "invalid sealed block file, try switch to parallel mode"
              << std::endl;
    is_sealed_file = false;
  }

  if (is_sealed_file) {
    parser = std::make_shared<file_parser>(
        psource.get(), rtarget.get(), sealer_enclave_file, parser_enclave_file,
        keymgr_enclave_file, sealed_file);

    if (vm.count("extra-data-source")) {
      extra_data_source_t eds;
      try {
        eds = ypc::read_extra_data_source_from_file(
            vm["extra-data-source"].as<std::string>());
      } catch (const std::exception &e) {
        std::cout << "cannot read extra-data-source file path: "
                  << vm["extra-data-source"].as<std::string>();
        return -1;
      }
      parser->set_extra_data_source(eds);
    }

    uint32_t ret = parser->parse();
    if (ret) {
      std::cout << "got error: " << ypc::status_string(ret) << std::endl;
    }
  } else {
    if (vm.count("extra-data-source")) {
      // TODO we may support this later.
      std::cout << "do not support parallel mode with extra data source"
                << std::endl;
      return -1;
    }
    if (source_type != "json") {
      std::cout << "parallel parser now only supports file type!" << std::endl;
      return -1;
    }
    std::string result_output_file = vm["result-path"].as<std::string>();
    parallel_parse(psource, result_output_file, sealer_enclave_file,
                   parser_enclave_file, keymgr_enclave_file, sealed_file);
  }

  return 0;
}

void parallel_parse(std::shared_ptr<param_source> psource,
                    const std::string &result_output_file,
                    const std::string &sealer_enclave_file,
                    const std::string &parser_enclave_file,
                    const std::string &keymgr_enclave_file,
                    const std::string &sealed_file) {

  ypc::ntobject_file<sfm_t> nf(sealed_file);
  nf.read_from();

  const std::vector<sfm_item_t> &items = nf.data().get<sfm_items>();

  std::vector<pid_t> pids;
  pid_t pid;
  signal(SIGCHLD, SIG_IGN);
  bool continue_flag = true;
  std::shared_ptr<result_target> round_result_target =
      std::make_shared<result_to_json>(result_output_file);

  std::shared_ptr<param_source> tmp_source =
      std::make_shared<param_from_memory>(*psource);

  int round = 0;
  while (continue_flag) {
    pids.clear();
    round++;
    LOG(INFO) << " round " << round;

    std::vector<std::shared_ptr<result_target>> tmp_results;
    for (auto item : items) {
      std::string sf = item.get<sfm_path>();
      uint16_t index = item.get<sfm_index>();
      std::shared_ptr<result_target> crtarget =
          std::make_shared<result_to_json>(result_output_file +
                                           std::to_string(index));
      tmp_results.push_back(crtarget);

      pid = fork();
      if (pid < 0) {
        std::cout << "fork error for " << index << std::endl;
        exit(-1);
      } else if (pid == 0) {
        parser = std::make_shared<file_parser>(
            tmp_source.get(), crtarget.get(), sealer_enclave_file,
            parser_enclave_file, keymgr_enclave_file, sf);
        parser->parse();
        continue_flag = false;
        break;
      } else {
        pids.push_back(pid);
      }
    }
    if (pid == 0) {
      std::cout << "Child process done" << std::endl;
      return;
    }

    std::cout << "Done forking, waiting ... " << std::endl;
    int status = 0;
    wait(&status);

    std::cout << "Wait all block results, done!" << std::endl;
    pid = fork();
    if (pid < 0) {
      std::cout << "fork error for merge" << std::endl;
      exit(-1);
    } else if (pid == 0) {
      parser = std::make_shared<file_parser>(
          tmp_source.get(), round_result_target.get(), sealer_enclave_file,
          parser_enclave_file, keymgr_enclave_file,
          ""); // last param is no use for merge
      continue_flag = parser->merge(tmp_results);
      if (continue_flag) {
        exit(0x1312);
      } else {
        exit(0);
      }
    } else {
      wait(&status);
      auto val = WEXITSTATUS(status);
      continue_flag = (val == 0x1312);

      // make the result as new param
      ypc::bytes encrypted_result, result_sig, data_hash, cost_sig;
      round_result_target->read_from_target(encrypted_result, result_sig,
                                            cost_sig, data_hash);
      tmp_source->input() = encrypted_result;
    }
  }
}
