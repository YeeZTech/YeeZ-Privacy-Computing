#include "datahub.hpp"

int main(int argc, char *argv[]) {
  boost::program_options::variables_map vm;
  try {
    vm = parse_command_line(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << "invalid cmd line parameters!" << std::endl;
    return -1;
  }

  if (vm.count("crypto") == 0u) {
    std::cerr << "crypto not specified" << std::endl;
    return -1;
  }
  if (vm.count("data-url") == 0u) {
    std::cerr << "data not specified!" << std::endl;
    return -1;
  }
  if (vm.count("plugin-path") == 0u) {
    std::cerr << "library not specified" << std::endl;
    return -1;
  }
  if ((vm.count("use-publickey-hex") == 0u) && (vm.count("use-publickey-file") == 0u)) {
    std::cerr << "missing public key, use 'use-publickey-file' or "
                 "'use-publickey-hex'"
              << std::endl;
    return -1;
  }
  if (vm.count("sealed-data-url") == 0u) {
    std::cerr << "sealed data url not specified" << std::endl;
    return -1;
  }
  if (vm.count("output") == 0u) {
    std::cerr << "output not specified" << std::endl;
    return -1;
  }

  std::string crypto = vm["crypto"].as<std::string>();
  std::string data_file = vm["data-url"].as<std::string>();
  std::string plugin = vm["plugin-path"].as<std::string>();

  ypc::bytes public_key;
  if (vm.count("use-publickey-hex") != 0u) {
    public_key = ypc::hex_bytes(vm["use-publickey-hex"].as<std::string>())
                     .as<ypc::bytes>();
  } else if (vm.count("use-publickey-file") != 0u) {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(
        vm["use-publickey-file"].as<std::string>(), pt);
    public_key = pt.get<ypc::bytes>("public-key");
  }

  std::string sealed_data_file = vm["sealed-data-url"].as<std::string>();
  std::string output = vm["output"].as<std::string>();

  ypc::bytes data_hash;
  std::ofstream ofs;
  ofs.open(output);
  if (!ofs.is_open()) {
    std::cout << "Cannot open file " << output << "\n";
    return -1;
  }
  ofs.close();

  crypto_ptr_t crypto_ptr;
  ypc::bytes pkey_kgt;
  if (crypto == "stdeth") {
    crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::eth_sgx_crypto>>();
    pkey_kgt = gen_pkey_kgt<ypc::crypto::eth_sgx_crypto>(public_key);
  } else if (crypto == "gmssl") {
    crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::gmssl_sgx_crypto>>();
    pkey_kgt = gen_pkey_kgt<ypc::crypto::gmssl_sgx_crypto>(public_key);
  } else {
    throw std::runtime_error("Unsupperted crypto type!");
  }

  auto status = seal_file(crypto_ptr, plugin, data_file, sealed_data_file,
                          public_key, data_hash);
  if (status != 0u) {
    return -1;
  }

  ofs.open(output);
  if (!ofs.is_open()) {
    std::cout << "Cannot open file " << output << "\n";
    return -1;
  }
  ofs << "data_url"
      << " = " << data_file << "\n";
  ofs << "sealed_data_url"
      << " = " << sealed_data_file << "\n";
  ofs << "public_key"
      << " = " << public_key << "\n";
  ofs << "data_id"
      << " = " << data_hash << "\n";
  ofs << "pkey_kgt"
      << " = " << pkey_kgt << "\n";

  ypc::privacy_data_reader reader(plugin, data_file);
  ofs << "item_num"
      << " = " << reader.get_item_number() << "\n";

  // sample and format are optional
  ypc::bytes sample = reader.get_sample_data();
  if (!sample.empty()) {
    ofs << "sample_data"
        << " = " << sample << "\n";
  }
  std::string format = reader.get_data_format();
  if (!format.empty()) {
    ofs << " data_fromat"
        << " = " << format << "\n";
  }
  ofs.close();

  std::cout << "done sealing" << std::endl;
  return 0;
}
