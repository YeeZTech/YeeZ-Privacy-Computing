#pragma once
#include "ypc/core_t/analyzer/internal/is_multi_datasource.h"
#include "ypc/core_t/analyzer/var/data_hash_var.h"
#include "ypc/core_t/analyzer/var/data_source_var.h"
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/stx_status.h"

namespace ypc {
namespace internal {

template <typename Crypto, typename DataSession,
          bool has_multi_datasource = is_multi_datasource<DataSession>::value>
class data_hash_interface : virtual public data_hash_var,
                            virtual public data_source_var<DataSession> {
protected:
  void set_data_hash() {
    stbox::bytes joint_bytes;
    std::vector<stbox::bytes> rs;
    for (uint32_t i = 0; i < data_source_var<DataSession>::m_datasource.size();
         ++i) {
      auto t = data_source_var<DataSession>::m_datasource[i]->data_hash();
      rs.push_back(t);
    }

    std::sort(
        rs.begin(), rs.end(),
        [](const stbox::bytes &a, const stbox::bytes &b) { return a > b; });
    rs.erase(std::unique(rs.begin(), rs.end()), rs.end());

    for (uint32_t i = 0; i < rs.size(); ++i) {
      joint_bytes += rs[i];
      LOG(INFO) << i << "-th data with data hash: " << rs[i];
    }

    Crypto::hash_256(joint_bytes, data_hash_var::m_data_hash);
#ifdef DEBUG
    LOG(INFO) << "final data hash: " << data_hash_var::m_data_hash;
#endif
  }
};

template <typename Crypto>
class data_hash_interface<Crypto, noinput_data_stream, false> {
protected:
  void set_data_hash() {}
};

template <typename Crypto, typename DataSession>
class data_hash_interface<Crypto, DataSession, false>
    : virtual public data_hash_var,
      virtual public data_source_var<DataSession> {
  typedef data_source_var<DataSession> data_source_var_t;

protected:
  void set_data_hash() {
    data_hash_var::m_data_hash = data_source_var_t::m_datasource->data_hash();
  }
};

template <typename Crypto>
class data_hash_interface<Crypto, oram_sealed_data_stream, false>
    : virtual public data_merkle_hash_var,
      virtual public data_source_var<oram_sealed_data_stream> {
  typedef data_source_var<oram_sealed_data_stream> data_source_var_t;

protected:
  void set_data_hash() {
    std::vector<stbox::bytes> data_hash_array = data_source_var_t::m_datasource->data_hash();
    for(uint32_t i = 0; i < data_hash_array.size(); ++i) {
      data_merkle_hash_var::m_data_hash.push_back(data_hash_array[i]);
    }
  }
};

} // namespace internal
} // namespace ypc
