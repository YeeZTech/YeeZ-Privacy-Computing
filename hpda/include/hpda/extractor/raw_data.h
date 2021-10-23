#pragma once
#include <ff/util/ntobject.h>
#include <hpda/common/common.h>
#include <hpda/common/stream_policy.h>
#include <hpda/extractor/extractor_base.h>

namespace hpda {
namespace extractor {
namespace internal {
template <typename OutputObjType>
class raw_data_impl : public extractor_base<OutputObjType> {
public:
  raw_data_impl() : m_data(), m_index(-1), m_no_init(true){};
  virtual ~raw_data_impl() {}

  void add_data(const OutputObjType &obj) {
    m_data.push_back(obj);
  }

  virtual bool process() {
    if (m_data.empty()) {
      return false;
    }
    m_index++;

    int s = static_cast<int>(m_data.size());
    if (m_index >= s) {
      return false;
    }
    return true;
  }

  virtual OutputObjType output_value() {
    if (m_data.size() <= m_index) {
      throw std::runtime_error("no more data in raw_data ");
    }
    return m_data[m_index];
  }

protected:
  std::vector<OutputObjType> m_data;
  int32_t m_index;
  bool m_no_init;
};

} // namespace internal

template <typename... ARGS>
using raw_data = internal::raw_data_impl<ntobject<ARGS...>>;

} // namespace extractor
} // namespace hpda
