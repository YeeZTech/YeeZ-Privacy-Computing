#ifndef YEEZ_PRIVACY_COMPUTING_DATAHUB_H
#define YEEZ_PRIVACY_COMPUTING_DATAHUB_H

#include "ypc/common/crypto_prefix.h"
#include "ypc/common/limits.h"
#include "ypc/core/kgt_json.h"
#include "ypc/core/ntobject_file.h"
#include "ypc/core/privacy_data_reader.h"
#include "ypc/core/sealed_file.h"
#include "ypc/core/version.h"
#include "ypc/corecommon/crypto/gmssl.h"
#include "ypc/corecommon/crypto/stdeth.h"
#include "ypc/corecommon/kgt.h"
#include "ypc/corecommon/nt_cols.h"

#include <boost/program_options.hpp>
#include <boost/progress.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <iostream>

class crypto_base {
public:
  virtual uint32_t encrypt_message_with_prefix(const ypc::bytes &public_key,
                                               const ypc::bytes &data,
                                               uint32_t prefix,
                                               ypc::bytes &cipher) = 0;
  virtual uint32_t hash_256(const ypc::bytes &msg, ypc::bytes &hash) = 0;
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
  virtual uint32_t hash_256(const ypc::bytes &msg, ypc::bytes &hash) {
    return crypto_t::hash_256(msg, hash);
  }
};

template <typename Crypto> struct group_traits {};
template <> struct group_traits<ypc::crypto::eth_sgx_crypto> {
  using group_t = ypc::crypto::secp256k1_pkey_group;
};
template <> struct group_traits<ypc::crypto::gmssl_sgx_crypto> {
  using group_t = ypc::crypto::sm2_pkey_group;
};

void write_batch(const crypto_ptr_t &crypto_ptr, ypc::simple_sealed_file &sf,
                 const std::vector<ypc::bytes> &batch,
                 const ypc::bytes &public_key) {
  using ntt = ypc::nt<ypc::bytes>;
  ntt::batch_data_pkg_t pkg;
  ypc::bytes s;
  ypc::bytes batch_str =
      ypc::make_bytes<ypc::bytes>::for_package<ntt::batch_data_pkg_t,
                                               ntt::batch_data>(batch);
  uint32_t status = crypto_ptr->encrypt_message_with_prefix(
      public_key, batch_str, ypc::utc::crypto_prefix_arbitrary, s);
  if (status != 0u) {
    std::stringstream ss;
    ss << "encrypt "
       << " data fail: " << stbox::status_string(status);
    LOG(ERROR) << ss.str();
    std::cerr << ss.str();
    exit(1);
  }
  sf.write_item(s);
}

uint32_t seal_file(const crypto_ptr_t &crypto_ptr, const std::string &plugin,
                   const std::string &file, const std::string &sealed_file_path,
                   const ypc::bytes &public_key, ypc::bytes &data_hash) {
  // Read origin file use sgx to seal file
  ypc::privacy_data_reader reader(plugin, file);
  ypc::simple_sealed_file sf(sealed_file_path, false);
  // std::string k(file);
  // k = k + std::string(sealer_path);

  // magic string here!
  crypto_ptr->hash_256(ypc::bytes("Fidelius"), data_hash);

  ypc::bytes item_data = reader.read_item_data();
  if (item_data.size() > ypc::utc::max_item_size) {
    std::cerr << "only support item size that smaller than "
              << ypc::utc::max_item_size << " bytes!" << std::endl;
    return 1;
  }
  uint64_t item_number = reader.get_item_number();

  std::cout << "Reading " << item_number << " items ..." << std::endl;
  boost::progress_display pd(item_number);
  uint counter = 0;
  std::vector<ypc::bytes> batch;
  size_t batch_size = 0;
  while (!item_data.empty() && counter < item_number) {
    batch.push_back(item_data);
    batch_size += item_data.size();
    if (batch_size >= ypc::utc::max_item_size) {
      write_batch(crypto_ptr, sf, batch, public_key);
      batch.clear();
      batch_size = 0;
    }

    ypc::bytes k = data_hash + item_data;
    crypto_ptr->hash_256(k, data_hash);

    item_data = reader.read_item_data();
    if (item_data.size() > ypc::utc::max_item_size) {
      std::cerr << "only support item size that smaller than "
                << ypc::utc::max_item_size << " bytes!" << std::endl;
      return 1;
    }
    ++pd;
    ++counter;
  }
  if (!batch.empty()) {
    write_batch(crypto_ptr, sf, batch, public_key);
    batch.clear();
    batch_size = 0;
  }

  std::cout << "data hash: " << data_hash << std::endl;
  std::cout << "\nDone read data count: " << pd.count() << std::endl;
  return 0;
}

template <typename Crypto>
ypc::bytes gen_pkey_kgt(const ypc::bytes &public_key) {
  using group_t = typename group_traits<Crypto>::group_t;
  using key_t = typename group_t::key_t;
  key_t key;
  memcpy(&key, public_key.data(), public_key.size());
  auto kn_ptr = std::make_shared<ypc::key_node<group_t>>(key);
  ypc::kgt<group_t> pkey_kgt(kn_ptr);
  return pkey_kgt.to_bytes();
}

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("YeeZ Privacy DataHub Options");
  bp::options_description general("General Options");
  bp::options_description seal_data_opts("Seal Data Options");

  // clang-format off
  seal_data_opts.add_options()
    ("crypto", bp::value<std::string>()->default_value("stdeth"), "choose the crypto, stdeth/gmssl")
    ("data-url", bp::value<std::string>(), "data url")
    ("plugin-path", bp::value<std::string>(), "shared library for reading data")
    ("use-publickey-file", bp::value<std::string>(), "public key file")
    ("use-publickey-hex", bp::value<std::string>(), "public key")
    ("sealed-data-url", bp::value<std::string>(), "sealed data url")
    ("output", bp::value<std::string>(), "output meta file path");

  general.add_options()
    ("help", "help message")
    ("version", "show version");
  // clang-format on

  all.add(general).add(seal_data_opts);
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

#endif //YEEZ_PRIVACY_COMPUTING_DATAHUB_H
