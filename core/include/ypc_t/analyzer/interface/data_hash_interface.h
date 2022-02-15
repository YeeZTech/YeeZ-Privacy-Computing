#pragma once
#include "stbox/ebyte.h"
#include "stbox/stx_status.h"
#include "ypc_t/analyzer/internal/is_multi_datasource.h"
#include "ypc_t/analyzer/var/data_hash_var.h"
#include "ypc_t/analyzer/var/data_source_var.h"

namespace ypc {
namespace internal {

template <typename DataSession,
          bool has_multi_datasource = is_multi_datasource<DataSession>::value>
class data_hash_interface : virtual public data_hash_var,
                            virtual public data_source_var<DataSession> {
protected:
  stbox::bytes set_data_hash() {
    // TODO
    return stbox::bytes();
  }
};

template <> class data_hash_interface<noinput_data_stream, false> {
protected:
  void set_data_hash() {}
};

template <typename DataSession>
class data_hash_interface<DataSession, false>
    : virtual public data_hash_var,
      virtual public data_source_var<DataSession> {
  typedef data_source_var<DataSession> data_source_var_t;

protected:
  void set_data_hash() {
    data_hash_var::m_data_hash = data_source_var_t::m_datasource->data_hash();
  }
};

} // namespace internal
} // namespace ypc
