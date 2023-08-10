#include "ypc/common/crypto_prefix.h"
#include "ypc/common/limits.h"
#include "ypc/core/ntobject_file.h"
#include "ypc/core/privacy_data_reader.h"
#include "ypc/core/oram_sealed_file.h"
#include "ypc/core/version.h"
#include "ypc/corecommon/crypto/gmssl.h"
#include "ypc/corecommon/crypto/stdeth.h"
#include "ypc/corecommon/nt_cols.h"
#include "ypc/corecommon/oram_types.h"
#include <boost/program_options.hpp>
#include <boost/progress.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <exception>
#include <fstream>
#include <iostream>
#include <thread>
#include <random>

using stx_status = stbox::stx_status;
using namespace ypc;
using ntt = ypc::nt<ypc::bytes>;
using oram_ntt = ypc::oram::nt<ypc::bytes>;
bool flag = false;

define_nt(input_buf, std::string);
typedef ff::net::ntpackage<0, input_buf> input_buf_t;

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

ypc::bytes random_string(size_t len) {
  std::string ret(len, '0');
  static std::default_random_engine generator;
  static std::uniform_int_distribution<int> distribution(int('a'), int('z'));
  static auto rand = std::bind(distribution, generator);

  for (size_t i = 0; i < len; i++) {
    ret[i] = rand();
  }
  return ypc::bytes(ret.data(), ret.size());
}

void push_dummy_block(std::vector<oram_ntt::block_t>& bucket_array, ypc::bytes &data_hash,
                      uint8_t count, uint64_t item_num_each_batch, size_t item_size,
                      const crypto_ptr_t &crypto_ptr, const ypc::bytes &public_key) {
  for(uint8_t i = 0; i < count; ++i) {
    oram_ntt::block_t b_block;

    std::vector<ypc::bytes> dummy_batch;
    for(uint32_t j = 0; j < item_num_each_batch; ++j) {
      ypc::bytes dummy_item = random_string(item_size);
      dummy_batch.push_back(dummy_item);
      ypc::bytes k_hash = data_hash + dummy_item;
      crypto_ptr->hash_256(k_hash, data_hash);
    }

    bytes encrypted_dummy_batch;
    ypc::bytes dummy_batch_str =
      ypc::make_bytes<ypc::bytes>::for_package<ntt::batch_data_pkg_t,
                                               ntt::batch_data>(dummy_batch);

    // encrypt dummy batch
    uint32_t status = crypto_ptr->encrypt_message_with_prefix(
      public_key, dummy_batch_str, ypc::utc::crypto_prefix_arbitrary, encrypted_dummy_batch);
    if (status != 0u) {
      std::stringstream ss;
      ss << "encrypt "
        << " data fail: " << stbox::status_string(status);
      LOG(ERROR) << ss.str();
      std::cerr << ss.str();
      exit(1);
    }

    b_block.set<oram_ntt::block_id, oram_ntt::leaf_label, oram_ntt::valid_item_num, oram_ntt::encrypted_batch>(0, 0, 0, encrypted_dummy_batch);
    bucket_array.push_back(b_block);
  }
}

uint32_t get_leaf_label(uint32_t bucket_index, uint8_t level_num_L) {
  // leftmost leaf node
  uint32_t leftmost_leaf_index = (1 << level_num_L) - 1;
  if(bucket_index >= leftmost_leaf_index) {
      return bucket_index - leftmost_leaf_index + 1;
  }

  // randomly select a path to the leaf node
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 1);

  if(dis(gen) == 0) {
      return get_leaf_label(2 * bucket_index + 1, level_num_L);
  }
  return get_leaf_label(2 * bucket_index + 2, level_num_L);
}

void push_real_block(std::vector<oram_ntt::block_t>& bucket_array, ypc::bytes &data_hash,
                      uint32_t& block_id_value, uint32_t bucket_index, 
                      std::vector<uint32_t> &position_map_array, uint8_t level_num_L,
                      std::vector<ypc::bytes> &batch, uint32_t &batch_str_size,
                      uint64_t item_num_each_batch, size_t item_size, 
                      const crypto_ptr_t &crypto_ptr, const ypc::bytes &public_key) {
  oram_ntt::block_t b_block;
  uint32_t valid_item_num = batch.size();
  for(uint32_t i = 0; i < item_num_each_batch - valid_item_num; ++i) {
    ypc::bytes item = random_string(item_size);
    batch.push_back(item);
  }

  for(auto &item : batch) {
    ypc::bytes k_hash = data_hash + item;
    crypto_ptr->hash_256(k_hash, data_hash);
  }

  bytes encrypted_batch;
  ypc::bytes batch_str =
    ypc::make_bytes<ypc::bytes>::for_package<ntt::batch_data_pkg_t,
                                              ntt::batch_data>(batch);
  // encrypt batch
  uint32_t status = crypto_ptr->encrypt_message_with_prefix(
    public_key, batch_str, ypc::utc::crypto_prefix_arbitrary, encrypted_batch);
  if (status != 0u) {
    std::stringstream ss;
    ss << "encrypt "
      << " data fail: " << stbox::status_string(status);
    LOG(ERROR) << ss.str();
    std::cerr << ss.str();
    exit(1);
  }

  if(batch_str_size != encrypted_batch.size()) {
    batch_str_size = encrypted_batch.size();
  }

  uint32_t b_leaf_label = get_leaf_label(bucket_index, level_num_L);
  position_map_array[block_id_value] = b_leaf_label;
  b_block.set<oram_ntt::block_id, oram_ntt::leaf_label, oram_ntt::valid_item_num, oram_ntt::encrypted_batch>(block_id_value++, b_leaf_label, valid_item_num, encrypted_batch);
  bucket_array.push_back(b_block);

}

uint32_t seal_oram_file(const crypto_ptr_t &crypto_ptr, const std::string &plugin,
                   const std::string &file, const std::string &oram_sealed_file_path,
                   const ypc::bytes &public_key) {
  // Read origin file use sgx to seal file
  privacy_data_reader reader(plugin, file);
  // std::string k(file);
  // k = k + std::string(sealer_path);

  // magic string here!

  bytes item_data = reader.read_item_data();
  if (item_data.size() > ypc::utc::max_item_size) {
    std::cerr << "only support item size that smaller than "
              << ypc::utc::max_item_size << " bytes!" << std::endl;
    return 1;
  }
  uint64_t item_number = reader.get_item_number();

  std::cout << "Reading " << item_number << " items ..." << std::endl;
  boost::progress_display pd(item_number);
  uint counter = 0;
  size_t batch_size = 0;
  size_t item_size = item_data.size();

  // the number of batch 
  uint64_t batch_num = 0;
  // item_num_array records the number of items each batch contains
  std::vector<uint64_t> item_num_array;
  uint64_t item_num_each_batch = 0;
  while (!item_data.empty() && counter < item_number) {
    ++item_num_each_batch;
    batch_size += item_data.size();
    if (batch_size >= ypc::utc::max_item_size) {
      item_num_array.push_back(item_num_each_batch);
      item_num_each_batch = 0;
      batch_size = 0;
      ++batch_num;
    }

    item_data = reader.read_item_data();
    if (item_data.size() > ypc::utc::max_item_size) {
      std::cerr << "only support item size that smaller than "
                << ypc::utc::max_item_size << " bytes!" << std::endl;
      return 1;
    }
    ++pd;
    ++counter;
  }

  if(item_num_each_batch != 0) {
    item_num_array.push_back(item_num_each_batch);
    item_num_each_batch = 0;
      batch_size = 0;
      ++batch_num;
  }
  
  assert(batch_num == item_num_array.size());


  // build id map
  LOG(INFO) << "build id_map";
  reader.reset_for_read();

  counter = 0;
  item_num_each_batch = 0;

  std::vector<oram_ntt::id_map_pair> id_map_array;
  uint32_t batch_id = 1;
  std::string item_index_field = reader.get_item_index_field();
  input_buf_t item_index_field_pkg;
  item_index_field_pkg.set<input_buf>(item_index_field);

  bytes item_index_field_bytes = make_bytes<bytes>::for_package(item_index_field_pkg);

  while (!item_index_field.empty() && counter < item_number) {
    bytes item_index_field_hash;
    crypto_ptr->hash_256(item_index_field_bytes, item_index_field_hash);

    oram_ntt::id_map_pair k_v;
    k_v.set<oram_ntt::item_index_field_hash, oram_ntt::block_id>(item_index_field_hash, batch_id);
    id_map_array.push_back(k_v);

    ++item_num_each_batch;
    if (item_num_each_batch >= item_num_array[batch_id-1]) {
      item_num_each_batch = 0;
      ++batch_id;
    }

    item_index_field = reader.get_item_index_field();
    input_buf_t item_index_field_pkg;
    item_index_field_pkg.set<input_buf>(item_index_field);
    item_index_field_bytes = make_bytes<bytes>::for_package(item_index_field_pkg);
    
    ++pd;
    ++counter;
  }

  assert(id_map_array.size() == item_number);
  oram_ntt::id_map_t id_map_pkg;
  id_map_pkg.set<oram_ntt::id_map>(id_map_array);
  bytes id_map_bytes = make_bytes<bytes>::for_package(id_map_pkg);
  

  // build header
  LOG(INFO) << "build header";
  oram::header osf_header{};
  osf_header.block_num = batch_num;
  uint32_t real_bucket_num = ceil(static_cast<double>(osf_header.block_num) / ypc::oram::BucketSizeZ);
  osf_header.level_num_L = ceil(log2(real_bucket_num + 1)) - 1; 
  osf_header.bucket_num_N = (1 << (osf_header.level_num_L + 1)) - 1;
  osf_header.id_map_filepos = sizeof(osf_header);

  osf_header.oram_tree_filepos = osf_header.id_map_filepos + id_map_bytes.size();
  // write header, id map, invalid position map
  LOG(INFO) << "write header, id map, invalid position map";
  std::fstream osf(oram_sealed_file_path, std::ios::out | std::ios::binary);
  if(!osf.is_open()) {
    throw std::runtime_error("Failed to create oram sealed file: " + oram_sealed_file_path);
  }

  osf.seekp(0, osf.beg);
  osf.write((char *)&osf_header, sizeof(osf_header));
  osf.write((char *)id_map_bytes.data(), id_map_bytes.size());


  std::vector<ypc::bytes> data_hash_array;


  // write ORAM tree
  item_num_each_batch = item_num_array.front();
  LOG(INFO) << "write ORAM tree";
  // from which bucket to start writing real blocks
  uint8_t lastbucket_realblocknum = osf_header.block_num % oram::BucketSizeZ;
  uint32_t bucket_index = 0; // bucket index in ORAM tree
  uint32_t block_id_value = 1; // block_id_value <= osf_header.block_num

  // write buckets full of dummy blocks
  LOG(INFO) << "write buckets full of dummy blocks";  
  osf.seekp(osf_header.oram_tree_filepos, osf.beg);
  for(uint32_t i = 0; i < osf_header.bucket_num_N - real_bucket_num; ++i) {
    std::vector<oram_ntt::block_t> bucket_array;
    ypc::bytes data_hash;
    crypto_ptr->hash_256(bytes("Fidelius"), data_hash);
    push_dummy_block(bucket_array, data_hash, oram::BucketSizeZ, 
        item_num_each_batch, item_size, crypto_ptr, public_key);
    oram_ntt::bucket_pkg_t bucket_pkg;
    bucket_pkg.set<oram_ntt::bucket>(bucket_array);
    bytes bucket_str = make_bytes<bytes>::for_package(bucket_pkg);

    // secondary encryption on the serialized bucket
    // in order to encrypt the mapping relationship between block_id and leaf_label
    bytes encrypted_bucket_bytes;
    uint32_t status = crypto_ptr->encrypt_message_with_prefix(
      public_key, bucket_str, ypc::utc::crypto_prefix_arbitrary, encrypted_bucket_bytes);
    if (status != 0u) {
      std::stringstream ss;
      ss << "encrypt "
        << " data fail: " << stbox::status_string(status);
      LOG(ERROR) << ss.str();
      std::cerr << ss.str();
      exit(1);
    }

    osf.write((char *)encrypted_bucket_bytes.data(), encrypted_bucket_bytes.size());

    data_hash_array.push_back(data_hash);

    ++bucket_index;
  }

  reader.reset_for_read();
  uint64_t batch_index = 0;
  std::vector<ypc::bytes> batch;
  std::vector<uint32_t> position_map_array(osf_header.block_num + 1, 0);

  // write the bucket that contains both real and dummy blocks
  LOG(INFO) << "write the bucket that contains both real and dummy blocks";
  if(lastbucket_realblocknum != 0) {
    --real_bucket_num;
    std::vector<oram_ntt::block_t> bucket_array;
    ypc::bytes data_hash;
    crypto_ptr->hash_256(bytes("Fidelius"), data_hash);
    push_dummy_block(bucket_array, data_hash, oram::BucketSizeZ - lastbucket_realblocknum, item_num_each_batch, item_size, crypto_ptr, public_key);
    for(int i = 0; i < lastbucket_realblocknum; ++i) {
      batch.clear();
      for(int j = 0; j < item_num_array[batch_index]; ++j) {
        item_data = reader.read_item_data();
        batch.push_back(item_data);
        ++pd;
      }
      push_real_block(bucket_array, data_hash, block_id_value, bucket_index, position_map_array, osf_header.level_num_L, batch, osf_header.batch_str_size, item_num_each_batch, item_size, crypto_ptr, public_key);
      ++batch_index;
    }    

    oram_ntt::bucket_pkg_t bucket_pkg;
    bucket_pkg.set<oram_ntt::bucket>(bucket_array);
    bytes bucket_str = make_bytes<bytes>::for_package(bucket_pkg);

    bytes encrypted_bucket_bytes;
    uint32_t status = crypto_ptr->encrypt_message_with_prefix(
      public_key, bucket_str, ypc::utc::crypto_prefix_arbitrary, encrypted_bucket_bytes);
    if (status != 0u) {
      std::stringstream ss;
      ss << "encrypt "
        << " data fail: " << stbox::status_string(status);
      LOG(ERROR) << ss.str();
      std::cerr << ss.str();
      exit(1);
    }

    osf.write((char *)encrypted_bucket_bytes.data(), encrypted_bucket_bytes.size());

    data_hash_array.push_back(data_hash);

    ++bucket_index;
  }

  // write buckets full of real blocks
  LOG(INFO) << "write buckets full of real blocks";
  for(uint32_t k = 0; k < real_bucket_num; ++k) {
    std::vector<oram_ntt::block_t> bucket_array;
    ypc::bytes data_hash;
    crypto_ptr->hash_256(bytes("Fidelius"), data_hash);
    for(int i = 0; i < oram::BucketSizeZ; ++i) {
      batch.clear();
      for(int j = 0; j < item_num_array[batch_index]; ++j) {
        item_data = reader.read_item_data();
        batch.push_back(item_data);
        ++pd;
      }
      push_real_block(bucket_array, data_hash, block_id_value, bucket_index, position_map_array, osf_header.level_num_L, batch, osf_header.batch_str_size, item_num_each_batch, item_size, crypto_ptr, public_key);
      ++batch_index;
    }

    oram_ntt::bucket_pkg_t bucket_pkg;
    bucket_pkg.set<oram_ntt::bucket>(bucket_array);
    bytes bucket_str = make_bytes<bytes>::for_package(bucket_pkg);

    bytes encrypted_bucket_bytes;
    uint32_t status = crypto_ptr->encrypt_message_with_prefix(
      public_key, bucket_str, ypc::utc::crypto_prefix_arbitrary, encrypted_bucket_bytes);
    if (status != 0u) {
      std::stringstream ss;
      ss << "encrypt "
        << " data fail: " << stbox::status_string(status);
      LOG(ERROR) << ss.str();
      std::cerr << ss.str();
      exit(1);
    }

    if(osf_header.bucket_str_size != encrypted_bucket_bytes.size()) {
      osf_header.bucket_str_size = encrypted_bucket_bytes.size();
    }

    osf.write((char *)encrypted_bucket_bytes.data(), encrypted_bucket_bytes.size());

    data_hash_array.push_back(data_hash);

    ++bucket_index;
  }

  // write position_map
  osf_header.position_map_filepos = osf.tellp();
  oram_ntt::position_map_t position_map_pkg;
  position_map_pkg.set<oram_ntt::position_map>(position_map_array);
  bytes position_map_bytes = make_bytes<bytes>::for_package(position_map_pkg);
  ypc::bytes encrypted_position_map_bytes;
  uint32_t status = crypto_ptr->encrypt_message_with_prefix(
      public_key, position_map_bytes, ypc::utc::crypto_prefix_arbitrary, encrypted_position_map_bytes);
  if (status != 0u) {
    std::stringstream ss;
    ss << "encrypt "
      << " data fail: " << stbox::status_string(status);
    LOG(ERROR) << ss.str();
    std::cerr << ss.str();
    exit(1);
  }

  osf.seekp(osf_header.position_map_filepos, osf.beg);
  osf.write((char *)encrypted_position_map_bytes.data(), encrypted_position_map_bytes.size());


  // write merkle tree
  assert(data_hash_array.size() == osf_header.bucket_num_N);
  osf_header.merkle_tree_filepos = osf.tellp();

  for(int i = (1 << osf_header.level_num_L) - 2; i >= 0; --i) {
    ypc::bytes k_hash = data_hash_array[i] + data_hash_array[2*i + 1];
    crypto_ptr->hash_256(k_hash, data_hash_array[i]);

    k_hash = data_hash_array[i] + data_hash_array[2*i + 2];
    crypto_ptr->hash_256(k_hash, data_hash_array[i]);
  }

  osf.seekp(osf_header.merkle_tree_filepos, osf.beg);
  for(auto &data_hash : data_hash_array) {
    osf.write((char *)data_hash.data(), data_hash.size());
  }


  osf_header.stash_filepos = osf.tellp();

  // update and write osf_header
  osf.seekp(0, osf.beg);
  osf.write((char *)&osf_header, sizeof(osf_header));

  osf.close();


  return 0;
}

boost::program_options::variables_map parse_command_line(int argc,
                                                         char *argv[]) {
  namespace bp = boost::program_options;
  bp::options_description all("YeeZ Privacy Data Hub options");
  bp::options_description general("General Options");
  bp::options_description seal_data_opts("Seal Data Options");

  // clang-format off
  seal_data_opts.add_options()
    ("crypto", bp::value<std::string>()->default_value("stdeth"), "choose the crypto, stdeth/gmssl")
    ("data-url", bp::value<std::string>(), "Data URL")
    ("plugin-path", bp::value<std::string>(), "shared library for reading data")
    ("use-publickey-file", bp::value<std::string>(), "public key file")
    ("use-publickey-hex", bp::value<std::string>(), "public key")
    ("sealed-data-url", bp::value<std::string>(), "Sealed data URL")
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

int main(int argc, char *argv[]) {
  boost::program_options::variables_map vm;
  try {
    vm = parse_command_line(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << "invalid cmd line parameters!" << std::endl;
    return -1;
  }
  if (vm.count("data-url") == 0u) {
    std::cerr << "data not specified!" << std::endl;
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
  if (vm.count("crypto") == 0u) {
    std::cerr << "crypto not specified" << std::endl;
    return -1;
  }

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

  std::string plugin = vm["plugin-path"].as<std::string>();
  std::string data_file = vm["data-url"].as<std::string>();
  std::string output = vm["output"].as<std::string>();
  std::string oram_sealed_data_file = vm["sealed-data-url"].as<std::string>();
  std::string crypto = vm["crypto"].as<std::string>();

  std::ofstream ofs;
  ofs.open(output);
  if (!ofs.is_open()) {
    std::cout << "Cannot open file " << output << "\n";
    return -1;
  }
  ofs.close();

  crypto_ptr_t crypto_ptr;
  if (crypto == "stdeth") {
    crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::eth_sgx_crypto>>();
  } else if (crypto == "gmssl") {
    crypto_ptr = std::make_shared<crypto_tool<ypc::crypto::gmssl_sgx_crypto>>();
  } else {
    throw std::runtime_error("Unsupperted crypto type!");
  }

  auto status = seal_oram_file(crypto_ptr, plugin, data_file, oram_sealed_data_file,
                          public_key);
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
      << " = " << oram_sealed_data_file << "\n";
  ofs << "public_key"
      << " = " << public_key << "\n";

  privacy_data_reader reader(plugin, data_file);
  ofs << "item_num"
      << " = " << reader.get_item_number() << "\n";

  // sample and format are optional
  bytes sample = reader.get_sample_data();
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
