#pragma once
#include "hpda/processor/processor_base.h"
#include "stbox/ebyte.h"
#include "stbox/eth/eth_hash.h"
#include "stbox/stx_common.h"
#include "stbox/tsgx/channel/dh_session_initiator.h"
#include "ypc_t/analyzer/ntpackage_item_parser.h"
#include "ypc_t/ecommon/package.h"
#include <ff/util/ntobject.h>

namespace ypc{

using bytes = ::stbox::bytes;
typedef ::ff::util::ntobject<nt<bytes>::data> data_source_output_t;
template <typename NTObj>
class to_type
    : public ::hpda::processor::internal::processor_base<data_source_output_t,
                                                         NTObj> {
public:
  to_type(::hpda::internal::processor_with_output<data_source_output_t>
              *upper_stream)
      : hpda::processor::internal::processor_base<data_source_output_t, NTObj>(
            upper_stream) {}

  typedef ::hpda::processor::internal::processor_base<data_source_output_t,
                                                      NTObj>
      base;
  virtual bool process() {
    if (!base::has_input_value()) {
      return false;
    }

    using ntt = nt<stbox::bytes>;
    const stbox::bytes &data =
        base::input_value().template get<typename ntt::data>();
    try {

      m_data = ntpackage_item_parser<stbox::byte_t, NTObj>::parser(data.data(),
                                                                   data.size());
    } catch (const std::exception &e) {
      LOG(ERROR) << "failed to convert to type";
      return false;
    }
    base::consume_input_value();
    return true;
  }

  virtual NTObj output_value() { return m_data; }

protected:
  NTObj m_data;
};
}
