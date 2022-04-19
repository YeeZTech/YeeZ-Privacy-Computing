#pragma once
#include "corecommon/package.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <ff/net/middleware/ntpackage.h>
#include <ff/sql/mysql.hpp>
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
class mysql_reader {
public:
  virtual int reset_for_read() = 0;
  virtual int read_item_data(char *buf, int *len) = 0;
  virtual int close_item_reader() = 0;
  virtual int get_item_number() = 0;
};

template <typename T, typename Table>
class typed_mysql_reader : public mysql_reader {
public:
  typedef T item_t;

  // TODO we should add more mysql options here
  /// @extra_param should be a json string, like this
  // {url: "xxx", username:"xxx", password:"xxx", dbname="xxx", table{name:"xx",
  // schema:[]}}
  typed_mysql_reader(const std::string &extra_param)
      : m_extra_param(extra_param) {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(m_extra_param, pt);

    auto url = pt.get_child("url").get_value<std::string>();
    auto username = pt.get_child("username").get_value<std::string>();
    auto password = pt.get_child("password").get_value<std::string>();
    auto dbname = pt.get_child("dbname").get_value<std::string>();
    m_engine.reset(
        new ff::sql::mysql<ff::sql::cppconn>(url, username, password, dbname));
  }

  virtual int reset_for_read() {
    m_to_read_index = 0;
    return 0;
  }
  template <typename NTObjType> struct read_helper {};
  template <typename... ARGS>
  struct read_helper<::ff::util::ntobject<ARGS...>> {
    template <typename TableT, typename EngineT, typename RetT>
    static void read_item_data(EngineT *engine, RetT &ret) {
      ret = TableT::template select<ARGS...>(engine).eval();
    }
  };

  template <typename... ARGS> struct assign_helper {};
  template <typename ARGT> struct assign_helper<ARGT> {
    template <typename RT, typename ST>
    static void assign(RT &to, const ST &from) {
      to.template set<ARGT>(from.template get<ARGT>());
    }
  };

  template <typename ARGT1, typename ARGT2, typename... ARGS>
  struct assign_helper<ARGT1, ARGT2, ARGS...> {
    template <typename RT, typename ST>
    static void assign(RT &to, const ST &from) {
      to.template set<ARGT1>(from.template get<ARGT1>());
      assign_helper<ARGT2, ARGS...>::assign(to, from);
    }
  };
  template <typename RT> struct assign_package {};
  template <typename... ARGS>
  struct assign_package<::ff::util::ntobject<ARGS...>> {
    template <typename PT>
    static void to(PT &_to, const ::ff::util::ntobject<ARGS...> &from) {
      assign_helper<ARGS...>::assign(_to, from);
    }
  };

  virtual int read_item_data(char *buf, int *len) {
    typedef typename ypc::cast_obj_to_package<item_t>::type package_t;
    if (m_all_items.empty()) {
      read_all_items();
    }

    if (m_to_read_index >= m_all_items.size()) {
      return -1;
    }
    package_t v;
    assign_package<item_t>::to(v, m_all_items[m_to_read_index]);
    m_to_read_index++;

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
    m_all_items.clear();
    return 0;
  }
  virtual int get_item_number() {
    if (m_all_items.empty()) {
      read_all_items();
    }
    return static_cast<int>(m_all_items.size());
  }
  const item_t &item_at(size_t index) const { return m_all_items[index]; }

protected:
  void read_all_items() {
    read_helper<item_t>::template read_item_data<Table>(m_engine.get(),
                                                        m_all_items);
    m_to_read_index = 0;
  }

  const std::string m_extra_param;
  std::unique_ptr<::ff::sql::mysql<ff::sql::cppconn>> m_engine;
  typename Table::row_collection_type m_all_items;
  size_t m_to_read_index;
};
} // namespace plugins
} // namespace ypc

#define impl_mysql_reader(type)                                                \
  void *create_item_reader(const char *extra_param, int len) {                 \
    try {                                                                      \
      ypc::plugins::mysql_reader *reader =                                     \
          new type(std::string(extra_param, len));                             \
      return reader;                                                           \
    } catch (const std::exception &e) {                                        \
      std::cout << "create_item_reader got: " << e.what() << std::endl;        \
      return nullptr;                                                          \
    }                                                                          \
  }                                                                            \
  int reset_for_read(void *handle) {                                           \
    ypc::plugins::mysql_reader *reader = (ypc::plugins::mysql_reader *)handle; \
    return reader->reset_for_read();                                           \
  }                                                                            \
  int read_item_data(void *handle, char *buf, int *len) {                      \
    ypc::plugins::mysql_reader *reader = (ypc::plugins::mysql_reader *)handle; \
    return reader->read_item_data(buf, len);                                   \
  }                                                                            \
  int close_item_reader(void *handle) {                                        \
    ypc::plugins::mysql_reader *reader = (ypc::plugins::mysql_reader *)handle; \
    reader->close_item_reader();                                               \
    delete reader;                                                             \
    return 0;                                                                  \
  }                                                                            \
                                                                               \
  uint64_t get_item_number(void *handle) {                                     \
    ypc::plugins::mysql_reader *reader = (ypc::plugins::mysql_reader *)handle; \
    return reader->get_item_number();                                          \
  }

