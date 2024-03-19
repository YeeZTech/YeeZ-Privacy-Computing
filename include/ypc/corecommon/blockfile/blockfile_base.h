#pragma once
#include "ypc/common/limits.h"
#include "ypc/corecommon/blockfile/blockfile_interface.h"
#include "ypc/corecommon/blockfile/traits.h"
#include "ypc/corecommon/exceptions.h"
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#ifdef HPDA_DEBUG
#include <glog/logging.h>
#endif
#ifdef YPC_SGX
#include "ypc/stbox/tsgx/log.h"
#else
#include <glog/logging.h>

#endif

namespace ypc {
namespace internal {
template <typename Header_t, typename File_t, uint64_t MagicNumber_t,
          uint64_t VersionNumber_t, uint64_t BlockNumLimit_t,
          uint64_t ItemNumPerBlockLimit_t>
class blockfile_impl : public blockfile_interface {
public:
  const static uint64_t MagicNumber = MagicNumber_t;
  const static uint64_t VersionNumber = VersionNumber_t;
  const static uint64_t BlockNumLimit = BlockNumLimit_t;
  const static uint64_t ItemNumPerBlockLimit = ItemNumPerBlockLimit_t;
  const static uint64_t BlockSizeLimit =
      ItemNumPerBlockLimit * ypc::utc::max_item_size;
  enum { succ = 0, eof = 1, invalid_buf = 2, small_buf = 3 };
  using ftt = file_traits<File_t>;

  blockfile_impl()
      : m_file(), m_file_path(), m_header(), m_is_header_valid(false),
        m_is_block_info_valid(false), m_block_infos() {}

  blockfile_impl(const blockfile_impl &) = delete;
  blockfile_impl(blockfile_impl &&) = delete;
  blockfile_impl &operator=(const blockfile_impl &) = delete;
  blockfile_impl &operator=(blockfile_impl &&) = delete;
  virtual ~blockfile_impl() = default;

  virtual void open_for_read(const char *file_path) {
    if (m_file.is_open()) {
      throw std::runtime_error("already open");
    }
    m_file_path = std::string(file_path);
    m_file.open(file_path, ftt::in | ftt::binary);
    if (!m_file.is_open()) {
      throw file_open_failure(m_file_path, "blockfile::open_for_read");
    }
    reset_read_item();
  }
  virtual void open_for_write(const char *file_path) {
    if (m_file.is_open()) {
      throw std::runtime_error("already open");
    }
    m_file_path = std::string(file_path);
    m_file.open(file_path, ftt::out | ftt::binary);
    if (!m_file.is_open()) {
      throw file_open_failure(m_file_path, "blockfile::open_for_write");
    }
  }

  template <typename ByteType>
  int append_item(const ByteType *data, size_t len) {
    static_assert(sizeof(ByteType) == 1);
    return append_item((const char *)data, len);
  }

  virtual int append_item(const char *data, size_t len) {
    // TODO: Check if reach limit
    read_header();
    read_all_block_info();
    m_file.clear();
    block_info bi{};

    m_header.item_number++;
    m_header.magic_number = MagicNumber;
    m_header.version_number = VersionNumber;

    if (m_block_infos.empty()) {
      bi.start_item_index = 0;
      bi.end_item_index = 1;
      bi.start_file_pos = block_start_offset;
      bi.end_file_pos = bi.start_file_pos + len + sizeof(len);
      m_block_infos.push_back(bi);
      m_header.block_number++;
    } else {
      bi = m_block_infos.back();
      if (bi.end_item_index - bi.start_item_index >= ItemNumPerBlockLimit) {
        auto back = m_block_infos.back();
        bi.start_item_index = back.end_item_index;
        bi.end_item_index = bi.start_item_index + 1;
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
        sizeof(Header_t) + (m_block_infos.size() - 1) * sizeof(block_info);
    m_file.seekp(offset, ftt::beg);
    m_file.write((char *)&back, sizeof(back));
    m_file.seekp(0, ftt::beg);
    m_file.write((char *)&m_header, sizeof(m_header));

    m_file.seekp(back.end_file_pos - len - sizeof(len), ftt::beg);
    m_file.write((char *)&len, sizeof(len));
    m_file.write(data, len);
    return 0;
  }

  virtual void reset_read_item() {
    m_file.clear();
    read_header();
    read_all_block_info();
    m_file.seekg(block_start_offset, ftt::beg);
  }

  virtual int next_item(char *buf, size_t in_size, size_t &out_size) {
    if (m_file.eof()) {
      return eof;
    }
    size_t len;
    m_file.read((char *)&len, sizeof(len));
    if (m_file.eof()) {
      return eof;
    }
    if (buf == nullptr) {
      m_file.seekp(-sizeof(len), ftt::cur);
      return invalid_buf;
    }
    out_size = len;
    if (in_size < len) {
      m_file.seekp(-sizeof(len), ftt::cur);
      return small_buf;
    }

    m_file.read(buf, len);
    return succ;
  }

  virtual uint64_t item_number() {
    if (!m_is_header_valid) {
      read_header();
    }
    return m_header.item_number;
  }

  virtual void close() {
    m_is_header_valid = false;
    m_is_block_info_valid = false;
    m_block_infos.clear();
    m_file.close();
  }

  const File_t &file() const { return m_file; }
  File_t &file() { return m_file; }

protected:
  void read_header() {
    if (m_is_header_valid) {
      return;
    }
    auto prev = m_file.tellg();

    m_file.seekg(0, ftt::beg);
    m_file.read((char *)&m_header, sizeof(Header_t));
    if (!m_file.eof() && m_header.magic_number != MagicNumber) {
      throw invalid_blockfile();
    }
    m_is_header_valid = true;
    m_file.seekg(prev, ftt::beg);
  }
  void read_all_block_info() {
    read_header();
    if (m_is_block_info_valid) {
      return;
    }
    m_file.seekg(sizeof(Header_t), ftt::beg);
    for (size_t i = 0; i < m_header.block_number; ++i) {
      block_info bi{};
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
    block_info() = default;
    uint64_t start_item_index;
    uint64_t end_item_index; // not included
    long int start_file_pos;
    long int end_file_pos;
  };
  const static long int block_start_offset =
      sizeof(Header_t) + sizeof(block_info) * BlockNumLimit;

  File_t m_file;
  std::string m_file_path;
  Header_t m_header;
  bool m_is_header_valid;
  bool m_is_block_info_valid;
  std::vector<block_info> m_block_infos;
};
} // namespace internal
} // namespace ypc
