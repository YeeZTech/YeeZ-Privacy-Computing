#include "../enclave/user_type.h"
#include "ypc/core/byte.h"
#include "ypc/core/kgt_json.h"
#include "ypc/core/ntjson.h"
#include "ypc/core/version.h"
#include "ypc/corecommon/crypto/gmssl.h"
#include "ypc/corecommon/crypto/stdeth.h"
#include "ypc/corecommon/nt_cols.h"

#include <boost/program_options.hpp>
#include <fstream>

using ntt = ypc::nt<ypc::bytes>;
typedef ff::util::ntobject<ntt::pkey, ntt::private_key> key_pair_t;
define_nt(key_pair_list, std::vector<key_pair_t>);
typedef ff::util::ntobject<key_pair_list> key_pair_list_t;

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("Task Graph Result Decrypt Options");
  bp::options_description general("General Options");
  bp::options_description result_decrypt_opts("Result Decrypt Options");

  // clang-format off
  result_decrypt_opts.add_options()
    ("crypto", bp::value<std::string>(), "crypto, stdeth/gmssl")
    ("encrypted-result", bp::value<std::string>(), "encrypted result in hex")
    ("kgt-pkey", bp::value<std::string>(), "public key generate tree flattened in hex")
    ("key-json-file", bp::value<std::string>(), "all related keys in json file")
    ("output", bp::value<std::string>(), "output file");

  general.add_options()
    ("help", "Help Message")
    ("version", "Show Version");
  // clang-format on

  all.add(general).add(result_decrypt_opts);

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, all), vm);

  if (vm.count("help") != 0u) {
    std::cout << all << std::endl;
    exit(-1);
  }
  if (vm.count("version") != 0u) {
    std::cout << ypc::get_ypc_version() << std::endl;
    exit(-1);
  }
  return vm;
}

class crypto_base {
public:
  virtual uint32_t encrypt_message_with_prefix(const ypc::bytes &public_key,
                                               const ypc::bytes &data,
                                               uint32_t prefix,
                                               ypc::bytes &cipher) = 0;
  virtual uint32_t decrypt_message_with_prefix(const ypc::bytes &private_key,
                                               const ypc::bytes &cipher,
                                               uint32_t prefix,
                                               ypc::bytes &data) = 0;
};

using crypto_ptr_t = std::shared_ptr<crypto_base>;
template <typename Crypto> class crypto_tool : public crypto_base {
public:
  using crypto_t = Crypto;
  virtual uint32_t encrypt_message_with_prefix(const ypc::bytes &public_key,
                                               const ypc::bytes &data,
                                               uint32_t prefix,
                                               ypc::bytes &cipher) {
    return crypto_t::encrypt_message_with_prefix(public_key, data, prefix,
                                                 cipher);
  }
  virtual uint32_t decrypt_message_with_prefix(const ypc::bytes &private_key,
                                               const ypc::bytes &cipher,
                                               uint32_t prefix,
                                               ypc::bytes &data) {
    return crypto_t::decrypt_message_with_prefix(private_key, cipher, data,
                                                 prefix);
  }
};

template <typename Crypto> struct group_traits {};
template <> struct group_traits<ypc::crypto::eth_sgx_crypto> {
  using pkey_group_t = ypc::crypto::secp256k1_pkey_group;
  using skey_group_t = ypc::crypto::secp256k1_skey_group;
};
template <> struct group_traits<ypc::crypto::gmssl_sgx_crypto> {
  using pkey_group_t = ypc::crypto::sm2_pkey_group;
  using skey_group_t = ypc::crypto::sm2_skey_group;
};

template <typename Crypto>
ypc::bytes
gen_kgt_skey_b(const ypc::bytes &kgt_pkey_b,
               const std::unordered_map<ypc::bytes, ypc::bytes> &peer) {
  using pkey_group_t = typename group_traits<Crypto>::pkey_group_t;
  using skey_group_t = typename group_traits<Crypto>::skey_group_t;

  ypc::kgt_json<pkey_group_t> kgt_pkey(kgt_pkey_b);
  auto skey_node =
      kgt_pkey.construct_skey_kgt_with_pkey_kgt(kgt_pkey.root(), peer);
  ypc::kgt_json<skey_group_t> kgt_skey(skey_node);
  kgt_skey.calculate_kgt_sum();
  auto skey_sum = kgt_skey.sum();
  ypc::bytes sum((uint8_t *)&skey_sum, sizeof(skey_sum));
  return sum;
}

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
  if (vm.count("encrypted-result") == 0u) {
    std::cerr << "encrypted result not specified!" << std::endl;
    return -1;
  }
  if (vm.count("kgt-pkey") == 0u) {
    std::cerr << "kgt pkey not specified" << std::endl;
    return -1;
  }
  if (vm.count("key-json-file") == 0u) {
    std::cerr << "key json file not specified" << std::endl;
    return -1;
  }

  std::string crypto = vm["crypto"].as<std::string>();
  ypc::bytes encrypted_result =
      ypc::hex_bytes(vm["encrypted-result"].as<std::string>()).as<ypc::bytes>();
  ypc::bytes kgt_pkey_b =
      ypc::hex_bytes(vm["kgt-pkey"].as<std::string>()).as<ypc::bytes>();
  std::string key_json_file = vm["key-json-file"].as<std::string>();

  auto all_keys = ypc::ntjson::from_json_file<key_pair_list_t>(key_json_file);
  // construct skey kgt
  std::unordered_map<ypc::bytes, ypc::bytes> peer;
  for (auto &key : all_keys.get<::key_pair_list>()) {
    peer.insert(
        std::make_pair(key.get<ntt::pkey>(), key.get<ntt::private_key>()));
  }

  crypto_ptr_t crypto_ptr;
  ypc::bytes kgt_skey_b;
  if (crypto == "stdeth") {
    using crypto_t = ypc::crypto::eth_sgx_crypto;
    crypto_ptr = std::make_shared<crypto_tool<crypto_t>>();
    kgt_skey_b = gen_kgt_skey_b<crypto_t>(kgt_pkey_b, peer);
  } else if (crypto == "gmssl") {
    using crypto_t = ypc::crypto::gmssl_sgx_crypto;
    crypto_ptr = std::make_shared<crypto_tool<crypto_t>>();
    kgt_skey_b = gen_kgt_skey_b<crypto_t>(kgt_pkey_b, peer);
  } else {
    throw std::runtime_error("Unsupperted crypto type!");
  }

  ypc::bytes result;
  crypto_ptr->decrypt_message_with_prefix(kgt_skey_b, encrypted_result, 0x2,
                                          result);

  std::stringstream ss;
  auto pkg = ypc::make_package<ntt::batch_data_pkg_t>::from_bytes(result);
  typedef typename ypc::cast_obj_to_package<merged_item_t>::type package_t;
  for (auto &batch : pkg.get<ntt::batch_data>()) {
    auto it = ypc::make_package<package_t>::from_bytes(batch);
    std::string line;
    line += (it.get<::id>() + ",");
    line += (it.get<::shxydm>() + ",");
    line += (it.get<::qycm>() + ",");
    line += (it.get<::hylx>() + ",");
    line += (it.get<::qylx>() + ",");
    line += (it.get<::jxmc>() + ",");
    line += (it.get<::zcdz>() + ",");
    line += (it.get<::jydz>() + ",");
    line += (std::to_string(it.get<::tax>()) + ",");
    line += (std::to_string(it.get<::qjsr>()) + ",");
    line += (it.get<::year>() + "\n");
    ss << line;
  }

  if (vm.count("output") == 0u) {
    std::cout << ss.str() << std::endl;
  }
  std::string output = vm["output"].as<std::string>();
  std::ofstream ofs;
  ofs.open(output);
  ofs << ss.str();
  ofs.close();
  return 0;
}
