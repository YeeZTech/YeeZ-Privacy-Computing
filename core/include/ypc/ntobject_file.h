#pragma once
#include "corecommon/package.h"
#include "ypc/byte.h"
#include "ypc/exceptions.h"
#include "ypc/filesystem.h"
#include <ff/net/middleware/ntpackage.h>
#include <fstream>
#include <string>

namespace ypc {
namespace internal {

template <typename NtObjTy> class ntobject_file_base {
public:
  ntobject_file_base(const std::string &file_path) : m_path(file_path) {}

  inline NtObjTy &data() { return m_sfm_data; }

  inline const NtObjTy &data() const { return m_sfm_data; }

  void read_from() {
    if (!is_file_exists(m_path)) {
      throw file_not_found(m_path, "ntobject_file::read_from()");
    }

    std::fstream fs;
    fs.open(m_path, std::ios::in | std::ios::binary);

    fs.seekg(0, fs.end);
    size_t size = fs.tellg();
    fs.seekg(0, fs.beg);

    bytes buf(size);
    fs.read((char *)buf.data(), size);
    ff::net::marshaler m((const char *)buf.data(), size,
                         ff::net::marshaler::deserializer);
    m_sfm_data.arch(m);

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
    bytes buf(len);

    ff::net::marshaler sm((char *)buf.data(), len,
                          ff::net::marshaler::serializer);
    m_sfm_data.arch(sm);

    fs.write((const char *)buf.data(), len);
    fs.close();
  }

protected:
  NtObjTy m_sfm_data;
  std::string m_path;
};

template <typename NtObjTy> struct check_nt_type {
  constexpr static uint32_t value = 0;
};
template <typename... ARGS>
struct check_nt_type<::ff::util::ntobject<ARGS...>> {
  constexpr static uint32_t value = 1;
};
template <uint32_t PackageID, typename... ARGS>
struct check_nt_type<::ff::net::ntpackage<PackageID, ARGS...>> {
  constexpr static uint32_t value = 2;
};
} // namespace internal

template <typename NtObjTy,
          uint32_t t = internal::check_nt_type<NtObjTy>::value>
class ntobject_file {
};
template <typename NtObjTy>
class ntobject_file<NtObjTy, 1>
    : public internal::ntobject_file_base<
          typename cast_obj_to_package<NtObjTy>::type> {
  typedef internal::ntobject_file_base<
      typename cast_obj_to_package<NtObjTy>::type>
      base_t;

public:
  ntobject_file(const std::string &file_path) : base_t(file_path) {}

  inline NtObjTy &data() { return m_local_data; }

  inline const NtObjTy &data() const { return m_local_data; }
  void read_from() {
    base_t::read_from();
    m_local_data = base_t::m_sfm_data;
  }
  void write_to() {
    base_t::m_sfm_data = m_local_data;
    base_t::write_to();
  }

protected:
  NtObjTy m_local_data;
};

template <typename NtObjTy>
class ntobject_file<NtObjTy, 2> : public internal::ntobject_file_base<NtObjTy> {
  typedef internal::ntobject_file_base<NtObjTy> base_t;

public:
  ntobject_file(const std::string &file_path) : base_t(file_path) {}
};
template <typename NtObjTy> class ntobject_file<NtObjTy, 0> {
public:
  ntobject_file(const std::string &file_path) {
    static_assert(
        internal::check_nt_type<NtObjTy>::value != 0,
        "ntobject_file only support ff::util::ntobject and ff::net::ntpackage");
  };
};

} // namespace ypc
