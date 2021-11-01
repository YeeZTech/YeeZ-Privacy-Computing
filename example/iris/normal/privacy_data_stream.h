#pragma once
#include "common/limits.h"
#include "hpda/extractor/extractor_base.h"
#include "ypc/privacy_data_reader.h"
#include <ff/util/ntobject.h>

namespace ypc {
using bytes = ::stbox::bytes;
template <typename OutputObjType>
class privacy_data_stream
    : public ::hpda::extractor::internal::extractor_base<OutputObjType> {
public:
  typedef OutputObjType user_item_t;
  typedef user_item_t (*item_parser_t)(const stbox::bytes::byte_t *, size_t);

  template <typename FuncT>
  privacy_data_stream(const std::string &plugin_path,
                      const std::string &extra_param, const FuncT &&item_parser)
      : m_reader(new privacy_data_reader(plugin_path, extra_param)),
        m_item_parser(item_parser) {
    m_reader->reset_for_read();
  }

  virtual ~privacy_data_stream() {}

  virtual bool process() {
    auto raw_data = m_reader->read_item_data();
    if (raw_data.size() == 0) {
      return false;
    }

    m_item_parser(raw_data.data(), raw_data.size(), (void *)&m_data);

    return true;
  }

  virtual OutputObjType output_value() { return m_data.make_copy(); }

protected:

  user_item_t m_data;
  bool m_data_reach_end;
  std::function<int(const uint8_t *, int, void *)> m_item_parser;
  std::unique_ptr<privacy_data_reader> m_reader;
};
} // namespace ypc
