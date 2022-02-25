#pragma once
#include "ypc/exceptions.h"
#include "ypc/memref.h"
#include <cassert>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#ifdef HPDA_DEBUG
#include <glog/logging.h>
#endif

namespace ypc {

class invalid_blockfile : public std::exception {
public:
  virtual const char *what() { return "wrong magic number"; }
};
// This is only for append and read, you cannot change any existed data
template <uint64_t MagicNumber_t, uint64_t BlockNumLimit_t,
          uint64_t BlockSizeLimit_t>
class blockfile {
public:
  const static uint64_t MagicNumber = MagicNumber_t;
  const static uint64_t BlockSizeLimit = BlockSizeLimit_t;
  const static uint64_t BlockNumLimit = BlockNumLimit_t;

  blockfile()
      : m_file(), m_file_path(), m_header(), m_is_header_valid(false),
        m_is_block_info_valid(false), m_block_infos() {}

  virtual ~blockfile(){};

  void open_for_read(const char *file_path) {
    if (m_file.is_open()) {
      throw std::runtime_error("already open");
    }
    m_file_path = std::string(file_path);
    m_file.open(file_path, std::ios::in | std::ios::binary);
    if (!m_file.is_open()) {
      throw file_open_failure(m_file_path, "blockfile::open_for_read");
    }

    reset_read_item();
  }
  void open_for_write(const char *file_path) {
    if (m_file.is_open()) {
      throw std::runtime_error("already open");
    }
    m_file_path = std::string(file_path);
    m_file.open(file_path, std::ios::out | std::ios::binary);
    if (!m_file.is_open()) {
      throw file_open_failure(m_file_path, "blockfile::open_for_write");
    }
  }

  template <typename ByteType>
  int append_item(const ByteType *data, size_t len) {
    static_assert(sizeof(ByteType) == 1);
    return append_item((const char *)data, len);
  }

  int append_item(const char *data, size_t len) {
    // TODO: Check if reach limit
    read_header();
    read_all_block_info();
    m_file.clear();
    block_info bi;

    m_header.item_number++;
    m_header.magic_number = MagicNumber;

    if (m_block_infos.empty()) {
      bi.start_item_index = 0;
      bi.end_item_index = 1;
      bi.start_file_pos = block_start_offset;
      bi.end_file_pos = bi.start_file_pos + len + sizeof(len);
      m_block_infos.push_back(bi);
      m_header.block_number++;
    } else {
      bi = m_block_infos.back();
      if (bi.end_item_index - bi.start_item_index >= BlockSizeLimit) {
        auto back = m_block_infos.back();
        bi.start_item_index = back.end_item_index;
        bi.end_item_index = bi.start_item_index++;
        bi.start_file_pos = back.end_file_pos;
        bi.end_file_pos = bi.end_file_pos + len + sizeof(len);
        m_block_infos.push_back(bi);
        m_header.block_number++;
      } else {
        block_info &back = m_block_infos.back();
        back.end_item_index++;
        back.end_file_pos = back.end_file_pos + len + sizeof(len);
      }
    }
    block_info &back = m_block_infos.back();
    auto offset =
        sizeof(header) + (m_block_infos.size() - 1) * sizeof(block_info);
    m_file.seekp(offset, m_file.beg);
    m_file.write((char *)&back, sizeof(back));
    m_file.seekp(0, m_file.beg);
    m_file.write((char *)&m_header, sizeof(m_header));

    m_file.seekp(back.end_file_pos - len - sizeof(len), m_file.beg);
    m_file.write((char *)&len, sizeof(len));
    m_file.write(data, len);
    return 0;
  }

  void reset_read_item() {
    m_file.clear();
    read_header();
    read_all_block_info();
    m_file.seekg(block_start_offset, m_file.beg);
  }

  bool next_item(memref &s) {
    if (m_file.eof())
      return false;
    size_t len;
    m_file.read((char *)&len, sizeof(len));
    if (m_file.eof()) {
      return false;
    }
    if (!s.data()) {
      s.alloc(len);
    }
    if (s.size() < len) {
      s.dealloc();
      s.alloc(len);
    }

    m_file.read((char *)s.data(), len);
    s.size() = len;
    return true;
  }

  uint64_t item_number() {
    if (!m_is_header_valid) {
      read_header();
    }
    return m_header.item_number;
  }

  void close() {
    m_is_header_valid = false;
    m_is_block_info_valid = false;
    m_block_infos.clear();
    m_file.close();
  }

protected:
  void read_header() {
    if (m_is_header_valid)
      return;
    auto prev = m_file.tellg();

    m_file.seekg(0, m_file.beg);
    m_file.read((char *)&m_header, sizeof(header));
    if (!m_file.eof() && m_header.magic_number != MagicNumber) {
      throw invalid_blockfile();
    }
    m_is_header_valid = true;
    m_file.seekg(prev, m_file.beg);
  }
  void read_all_block_info() {
    read_header();
    if (m_is_block_info_valid) {
      return;
    }
    m_file.seekg(sizeof(header), m_file.beg);
    for (size_t i = 0; i < m_header.block_number; ++i) {
      block_info bi;
      m_file.read((char *)&bi, sizeof(bi));
      m_block_infos.push_back(bi);
    }
    m_is_block_info_valid = true;
  }

protected:
  struct header {
    uint64_t magic_number;
    uint64_t version_number;
    uint64_t block_number;
    uint64_t item_number;
  };
  struct block_info {
    //[start_item_index, end_item_index)
    uint64_t start_item_index;
    uint64_t end_item_index; // not included
    long int start_file_pos;
    long int end_file_pos;
  };
  const static long int block_start_offset =
      sizeof(header) + sizeof(block_info) * BlockNumLimit;

  std::fstream m_file;
  std::string m_file_path;
  header m_header;
  bool m_is_header_valid;
  bool m_is_block_info_valid;
  std::vector<block_info> m_block_infos;
};
} // namespace ypc
