#pragma once
#include "stbox/stx_common.h"
#include "ypc/exceptions.h"
#include "ypc/filesystem.h"
#include <ff/net/middleware/ntpackage.h>
#include <fstream>
#include <string>

namespace ypc {
template <typename NtObjTy> class ntobject_file {
public:
  ntobject_file(const std::string &file_path) : m_path(file_path) {}

  inline NtObjTy &data() { return m_sfm_data; }

  inline const NtObjTy &data() const { return m_sfm_data; }

  void read_from() {
    if (!is_file_exists(m_path)) {
      throw file_not_found(m_path, "ntobject_file::read_from()");
    }

    std::fstream fs;
    fs.open(m_path, std::ios::in | std::ios::binary);
    // if (!fs.is_open()) {
    // throw file_open_failure(m_path, "ntobject_file::read_from()");
    //}

    fs.seekg(0, fs.end);
    size_t size = fs.tellg();
    fs.seekg(0, fs.beg);

    char *buf = new char[size];
    memset(buf, 0, size);
    fs.read(buf, size);
    stbox::print_hex((uint8_t *)buf, size);
    ff::net::marshaler m(buf, size, ff::net::marshaler::deserializer);
    m_sfm_data.arch(m);
    delete[] buf;

    fs.close();
  }
  void write_to() {
    std::fstream fs;

    fs.open(m_path, std::ios::out | std::ios::binary);
    if (!fs.is_open()) {
      throw file_open_failure(m_path, "ntobject_file::write_to()");
    }
    ff::net::marshaler m(ff::net::marshaler::length_retriver);
    m_sfm_data.arch(m);
    size_t len = m.get_length();
    char *buf = new char[len];

    ff::net::marshaler sm(buf, len, ff::net::marshaler::serializer);
    m_sfm_data.arch(sm);

    fs.write(buf, len);
    delete[] buf;
    fs.close();
  }

protected:
  NtObjTy m_sfm_data;
  std::string m_path;
};
} // namespace ypc
