#pragma once
#include "corecommon/package.h"
#include "csv.h"
#include "csv_read_assign_helper.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <ff/util/ntobject.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void *create_item_reader(const char *extra_param, int len);

int reset_for_read(void *handle);
int read_item_data(void *handle, char *buf, int *len);
int close_item_reader(void *handle);
uint64_t get_item_number(void *handle);

#ifdef __cplusplus
}
#endif

namespace ypc {
namespace plugins {
class csv_reader {
public:
  virtual int reset_for_read() = 0;
  virtual int read_item_data(char *buf, int *len) = 0;
  virtual int close_item_reader() = 0;
  virtual int get_item_number() = 0;
};


template <typename NT> struct ntobject_size { const static size_t size = 0; };
template <typename T1> struct ntobject_size<::ff::util::ntobject<T1>> {
  const static size_t size = 1;
};
template <typename T1, typename... ARGS>
struct ntobject_size<::ff::util::ntobject<T1, ARGS...>> {
  const static size_t size =
      1 + ntobject_size<::ff::util::ntobject<ARGS...>>::size;
};

template <typename T>
class typed_csv_reader : public csv_reader {
public:
  typedef T item_t;

  // TODO we should add more csv options here
  /// @extra_param should be a json string, like this
  // {file_path: "xxx"}
  typed_csv_reader(const std::string &extra_param)
      : m_extra_param(extra_param), m_file_path(extra_param) {
    /*
    boost::property_tree::ptree pt;
    std::stringstream ss;
    ss << extra_param;
    boost::property_tree::json_parser::read_json(ss, pt);
    m_file_path = pt.get_child("file_path").get_value<std::string>();
*/
    m_stream.reset(new std::ifstream(m_file_path));
    if (!m_stream->is_open()) {
      throw std::runtime_error("file not exist");
    }
    m_reader.reset(
        new io::CSVReader<ntobject_size<item_t>::size>(m_file_path, *m_stream));
  }
  virtual int reset_for_read() {
    m_stream.reset(new std::ifstream(m_file_path));
    if (!m_stream->is_open()) {
      return -1;
    }
    m_reader.reset(
        new io::CSVReader<ntobject_size<item_t>::size>(m_file_path, *m_stream));
    return 0;
  }
  item_t read_typed_item() {
    typedef typename cast_obj_to_package<item_t>::type package_t;
    package_t v;
    internal::assign_helper<item_t>::read_row(m_reader.get(), v);
    item_t ret = v;
    return ret;
  }

  virtual int read_item_data(char *buf, int *len) {
    typedef typename cast_obj_to_package<item_t>::type package_t;
    package_t v;
    bool rv = internal::assign_helper<item_t>::read_row(m_reader.get(), v);
    if (!rv) {
      return 1;
    }
    if (len) {
      ff::net::marshaler lm(ff::net::marshaler::length_retriver);
      v.arch(lm);
      *len = static_cast<int>(lm.get_length());
    }
    if (buf) {
      ff::net::marshaler sm(buf, *len, ff::net::marshaler::serializer);
      v.arch(sm);
    }
    return 0;
  }
  virtual int close_item_reader() {
    m_reader.reset();
    m_stream.reset();
    return 0;
  }
  virtual int get_item_number() {
    std::ifstream s(m_file_path);
    io::CSVReader<ntobject_size<item_t>::size> r(m_file_path, s);

    while (r.next_line()) {
    }
    auto l = r.get_file_line();
    return l;
  }

protected:
  const std::string m_extra_param;
  std::string m_file_path;
  std::unique_ptr<std::ifstream> m_stream;
  std::unique_ptr<io::CSVReader<ntobject_size<item_t>::size>> m_reader;
};
} // namespace plugins
} // namespace ypc

#define impl_csv_reader(type)                                                  \
  void *create_item_reader(const char *extra_param, int len) {                 \
    try {                                                                      \
      ypc::plugins::csv_reader *reader =                                       \
          new type(std::string(extra_param, len));                             \
      return reader;                                                           \
    } catch (const std::exception &e) {                                        \
      std::cout << "create_item_reader got exception: " << e.what()            \
                << std::endl;                                                  \
      return nullptr;                                                          \
    }                                                                          \
  }                                                                            \
  int reset_for_read(void *handle) {                                           \
    ypc::plugins::csv_reader *reader = (ypc::plugins::csv_reader *)handle;     \
    return reader->reset_for_read();                                           \
  }                                                                            \
  int read_item_data(void *handle, char *buf, int *len) {                      \
    ypc::plugins::csv_reader *reader = (ypc::plugins::csv_reader *)handle;     \
    return reader->read_item_data(buf, len);                                   \
  }                                                                            \
  int close_item_reader(void *handle) {                                        \
    ypc::plugins::csv_reader *reader = (ypc::plugins::csv_reader *)handle;     \
    reader->close_item_reader();                                               \
    delete reader;                                                             \
    return 0;                                                                  \
  }                                                                            \
                                                                               \
  uint64_t get_item_number(void *handle) {                                     \
    ypc::plugins::csv_reader *reader = (ypc::plugins::csv_reader *)handle;     \
    return reader->get_item_number();                                          \
  }

