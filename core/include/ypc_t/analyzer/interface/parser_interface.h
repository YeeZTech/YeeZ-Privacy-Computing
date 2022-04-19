#pragma once
#include "ypc_t/analyzer/internal/data_streams/noinput_data_stream.h"
#include "ypc_t/analyzer/internal/is_multi_datasource.h"
#include "ypc_t/analyzer/var/data_source_var.h"
#include "ypc_t/analyzer/var/parser_var.h"

namespace ypc {
namespace internal {

template <typename DataSession, typename ParserT,
          bool has_multi_datasource = is_multi_datasource<DataSession>::value>
class parser_interface {};

template <typename ParserT>
class parser_interface<noinput_data_stream, ParserT, false>
    : virtual public parser_var<ParserT> {
  typedef parser_var<ParserT> parser_var_t;

public:
  uint32_t create_parser() {
    parser_var_t::m_engine.reset(new ::hpda::engine());
    parser_var_t::m_parser.reset(new ParserT());
    return stbox::stx_status::success;
  };
};

template <typename DataSession, typename ParserT>
class parser_interface<DataSession, ParserT, false>
    : virtual public data_source_var<DataSession>,
      virtual public parser_var<ParserT> {
  typedef parser_var<ParserT> parser_var_t;
  typedef data_source_var<DataSession> data_source_var_t;

public:
  uint32_t create_parser() {
    if (!data_source_var_t::m_datasource) {
      return stbox::stx_status::data_source_not_set;
    }
    parser_var_t::m_engine.reset(new ::hpda::engine());
    data_source_var_t::m_datasource->set_engine(parser_var_t::m_engine.get());
    parser_var_t::m_parser.reset(
        new ParserT(data_source_var_t::m_datasource.get()));
    return stbox::stx_status::success;
  };
};

template <typename DataSession, typename ParserT>
class parser_interface<DataSession, ParserT, true>
    : virtual public data_source_var<DataSession>,
      virtual public parser_var<ParserT> {
  typedef data_source_var<DataSession> data_source_var_t;
  typedef parser_var<ParserT> parser_var_t;

public:
  uint32_t create_parser() {
    if (data_source_var_t::m_datasource.empty()) {
      return stbox::stx_status::data_source_not_set;
    }
    parser_var_t::m_engine.reset(new ::hpda::engine());
    for (auto ds : data_source_var_t::m_datasource) {
      ds->set_engine(parser_var_t::m_engine.get());
    }

    parser_var_t::m_parser.reset(new ParserT(data_source_var_t::m_datasource));
    return stbox::stx_status::success;
  };
};
} // namespace internal

} // namespace ypc
