#pragma once
#include "ypc/core/byte.h"
#include "ypc/core/exceptions.h"
#include "ypc/core/memref.h"
#include "ypc/corecommon/oram_types.h"
#include "ypc/corecommon/package.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

using oram_ntt = ypc::oram::nt<ypc::bytes>;

namespace ypc {
namespace oram {

template <uint64_t OramBlockNumLimit_t, uint32_t OramBlockSizeLimit_t,
          uint8_t OramBucketSize_t>
class oramblockfile {
public:
  const static uint64_t BlockNumLimit = OramBlockNumLimit_t;
  const static uint32_t DataSizeB = OramBlockSizeLimit_t;
  const static uint8_t BucketSizeZ = OramBucketSize_t;
  const static uint32_t hash_size = 32;

  oramblockfile(const std::string &file_path)
      : m_file(), m_file_path(file_path), m_header(), m_id_map() {
    open_for_write();
  }

  oramblockfile(const oramblockfile &) = delete;
  oramblockfile(oramblockfile &&) = delete;
  oramblockfile &operator=(const oramblockfile &) = delete;
  oramblockfile &operator=(oramblockfile &&) = delete;
  virtual ~oramblockfile() = default;

  void open_for_read() {
    if (m_file.is_open()) {
      throw std::runtime_error("already open");
    }

    m_file.open(m_file_path, std::ios::in | std::ios::binary);
    if (!m_file.is_open()) {
      throw file_open_failure(m_file_path, "oramblockfile::open_for_read");
    }

    read_header();
  }

  void open_for_write() {
    if (m_file.is_open()) {
      throw std::runtime_error("already open");
    }

    m_file.open(m_file_path, std::ios::in | std::ios::out | std::ios::binary);
    if (!m_file.is_open()) {
      throw file_open_failure(m_file_path, "oramblockfile::open_for_write");
    }
    read_header();
  }

  void reset() {
    m_file.clear();
    read_header();
  }

  void read_header() {
    m_file.seekg(0, m_file.beg);
    m_file.read((char *)&m_header, sizeof(header));
  }

  void read_id_map() {
    m_file.seekg(m_header.id_map_filepos, m_file.beg);
    bytes id_map_str(m_header.position_map_filepos - m_header.id_map_filepos);
    m_file.read((char *)id_map_str.data(), id_map_str.size());

    auto id_map_pkg = make_package<oram_ntt::id_map_t>::from_bytes(id_map_str);
    auto id_map_array = id_map_pkg.get<oram_ntt::id_map>();

    for (const auto &element : id_map_array) {
      bytes item_index_field_hash =
          element.get<oram_ntt::item_index_field_hash>();
      uint32_t block_id = element.get<oram_ntt::block_id>();
      m_id_map.insert({item_index_field_hash, block_id});
    }
  }

  bool download_position_map(memref &posmap) {
    size_t len = m_header.merkle_tree_filepos - m_header.position_map_filepos;
    if (posmap.data() == nullptr) {
      posmap.alloc(len);
    }
    if (posmap.size() < len) {
      posmap.dealloc();
      posmap.alloc(len);
    }

    m_file.seekg(m_header.position_map_filepos, m_file.beg);
    m_file.read((char *)posmap.data(), len);
    posmap.size() = len;
    return true;
  }

  bool get_block_id(bytes &item_index_field_hash, uint32_t *block_id) {
    read_id_map();
    if (m_id_map.find(item_index_field_hash) == m_id_map.end()) {
      return false;
    }
    *block_id = m_id_map.at(item_index_field_hash);
    return true;
  }

  const uint32_t &get_block_num() const { return m_header.block_num; }

  const uint32_t &get_bucket_num() const { return m_header.bucket_num_N; }

  const uint8_t &get_level_num() const { return m_header.level_num_L; }

  const uint32_t &get_bucket_str_size() const {
    return m_header.bucket_str_size;
  }

  const uint32_t &get_batch_str_size() const { return m_header.batch_str_size; }

  bool update_position_map(uint8_t *position_map, uint32_t len) {
    read_header();
    m_file.clear();

    m_file.seekp(m_header.position_map_filepos, m_file.beg);
    m_file.write((char *)(position_map), len);
    return true;
  }

  bool download_path(uint32_t leaf, memref &en_path) {
    std::vector<uint32_t> offsets;
    leaf_to_offsets(leaf, offsets);

    std::vector<bytes> en_path_array;
    for (const uint32_t &offset : offsets) {
      bytes en_bucket_str(m_header.bucket_str_size);
      m_file.seekg(m_header.oram_tree_filepos +
                       offset * m_header.bucket_str_size,
                   m_file.beg);
      m_file.read((char *)en_bucket_str.data(), m_header.bucket_str_size);
      en_path_array.push_back(en_bucket_str);
    }

    oram_ntt::path_pkg_t path_pkg;
    path_pkg.set<oram_ntt::path>(en_path_array);
    bytes en_path_str = make_bytes<bytes>::for_package(path_pkg);

    size_t len = en_path_str.size();
    if (en_path.data() == nullptr) {
      en_path.alloc(len);
    }
    if (en_path.size() < len) {
      en_path.dealloc();
      en_path.alloc(len);
    }

    memcpy(en_path.data(), en_path_str.data(), len);
    en_path.size() = len;

    return true;
  }

  bool upload_path(uint32_t leaf, uint8_t *encrpypted_path, uint32_t len) {
    read_header();
    m_file.clear();

    bytes encrpypted_path_str(len);
    memcpy(encrpypted_path_str.data(), encrpypted_path, len);

    oram_ntt::path_pkg_t path_pkg =
        make_package<oram_ntt::path_pkg_t>::from_bytes(encrpypted_path_str);
    std::vector<bytes> bucket_str_array = path_pkg.get<oram_ntt::path>();

    std::vector<uint32_t> offsets;
    leaf_to_offsets(leaf, offsets);
    for (uint8_t i = 0; i < offsets.size(); ++i) {
      m_file.seekp(m_header.oram_tree_filepos +
                       offsets[i] * m_header.bucket_str_size,
                   m_file.beg);
      m_file.write((char *)bucket_str_array[i].data(),
                   m_header.bucket_str_size);
    }

    return true;
  }

  bool download_stash(memref &st) {
    if (m_header.stash_size == 0) {
      return true;
    }

    size_t len = m_header.stash_size;
    if (st.data() == nullptr) {
      st.alloc(len);
    }
    if (st.size() < len) {
      st.dealloc();
      st.alloc(len);
    }

    m_file.seekg(m_header.stash_filepos, m_file.beg);
    m_file.read((char *)st.data(), len);
    st.size() = len;
    return true;
  }

  bool update_stash(uint8_t *stash, uint32_t len) {
    read_header();
    m_file.clear();

    m_header.stash_size = len;
    m_file.seekp(0, m_file.beg);
    m_file.write((char *)&m_header, sizeof(m_header));

    if (len > 0) {
      m_file.seekp(m_header.stash_filepos, m_file.beg);
      m_file.write((char *)(stash), len);
    }

    return true;
  }

  void close() { m_file.close(); }

  bool read_root_hash(memref &root_hash) {
    size_t len = hash_size;
    if (root_hash.data() == nullptr) {
      root_hash.alloc(len);
    }
    if (root_hash.size() < len) {
      root_hash.dealloc();
      root_hash.alloc(len);
    }

    m_file.seekg(m_header.merkle_tree_filepos, m_file.beg);
    m_file.read((char *)root_hash.data(), len);
    root_hash.size() = len;

    return true;
  }

  bool download_merkle_hash(uint32_t leaf, memref &merkle_hash) {
    std::vector<uint32_t> hash_offsets;
    get_hash_offsets(leaf, hash_offsets);

    std::vector<uint32_t> path_offsets;
    leaf_to_offsets(leaf, path_offsets);

    std::vector<oram_ntt::hash_pair> merkle_hash_array;
    uint32_t i = 0;
    for (const uint32_t &offset : hash_offsets) {
      bytes data_hash(hash_size);
      bool in_path = false;
      m_file.seekg(m_header.merkle_tree_filepos + offset * hash_size,
                   m_file.beg);
      m_file.read((char *)data_hash.data(), hash_size);

      if (offset == path_offsets[i]) {
        ++i;
        in_path = true;
      }
      oram_ntt::hash_pair hash_p;
      hash_p.set<oram_ntt::data_hash, oram_ntt::in_path>(data_hash, in_path);
      merkle_hash_array.push_back(hash_p);
    }

    oram_ntt::merkle_hash_pkg_t merkle_hash_pkg;
    merkle_hash_pkg.set<oram_ntt::merkle_hash>(merkle_hash_array);
    bytes merkle_hash_str = make_bytes<bytes>::for_package(merkle_hash_pkg);

    size_t len = merkle_hash_str.size();
    if (merkle_hash.data() == nullptr) {
      merkle_hash.alloc(len);
    }
    if (merkle_hash.size() < len) {
      merkle_hash.dealloc();
      merkle_hash.alloc(len);
    }

    memcpy(merkle_hash.data(), merkle_hash_str.data(), len);
    merkle_hash.size() = len;

    return true;
  }

  bool update_merkle_hash(uint32_t leaf, uint8_t *merkle_hash, uint32_t len) {
    read_header();
    m_file.clear();

    bytes merkle_hash_str(len);
    memcpy(merkle_hash_str.data(), merkle_hash, len);

    oram_ntt::merkle_hash_pkg_t merkle_hash_pkg =
        make_package<oram_ntt::merkle_hash_pkg_t>::from_bytes(merkle_hash_str);
    auto merkle_hash_array = merkle_hash_pkg.get<oram_ntt::merkle_hash>();

    std::vector<uint32_t> offsets;
    get_hash_offsets(leaf, offsets);
    for (uint8_t i = 0; i < offsets.size(); ++i) {
      m_file.seekp(m_header.merkle_tree_filepos + offsets[i] * hash_size,
                   m_file.beg);
      m_file.write(
          (char *)merkle_hash_array[i].get<oram_ntt::data_hash>().data(),
          hash_size);
    }

    return true;
  }

private:
  void get_hash_offsets(uint32_t leaf, std::vector<uint32_t> &offsets) {
    if (leaf == 0) {
      throw std::runtime_error("leaf label is invalid!");
    }

    // Convert leaf to index in ORAM tree
    uint32_t cur_node = (1 << m_header.level_num_L) - 1 + leaf - 1;

    bool flag = true;
    while (flag) {
      if (cur_node == 0) {
        offsets.push_back(cur_node);
        break;
      }

      if (cur_node % 2 == 0) { // cur_node is a right child node
        offsets.push_back(cur_node);
        offsets.push_back(cur_node - 1);
      } else {
        offsets.push_back(cur_node + 1);
        offsets.push_back(cur_node);
      }

      cur_node = (cur_node - 1) / 2;
    }
    std::reverse(offsets.begin(), offsets.end());
  }

  void leaf_to_offsets(uint32_t leaf, std::vector<uint32_t> &offsets) {
    if (leaf == 0) {
      throw std::runtime_error("leaf label is invalid!");
    }

    // Convert leaf to index in ORAM tree
    uint32_t cur_node = (1 << m_header.level_num_L) - 1 + leaf - 1;

    bool flag = true;
    while (flag) {
      if (cur_node == 0)
        flag = false;

      offsets.push_back(cur_node);

      cur_node = (cur_node - 1) / 2;
    }
    std::reverse(offsets.begin(), offsets.end());
  }

  std::fstream m_file;
  std::string m_file_path;
  header m_header;
  std::unordered_map<bytes, uint32_t> m_id_map;
};

} // namespace oram

} // namespace ypc
