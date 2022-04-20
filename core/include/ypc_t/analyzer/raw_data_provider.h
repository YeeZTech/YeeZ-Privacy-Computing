#pragma once
#include "common/limits.h"
#include "corecommon/package.h"
#include "eparser_t.h"
#include "hpda/extractor/extractor_base.h"
#include "stbox/ebyte.h"
#include "stbox/eth/eth_hash.h"
#include "stbox/stx_common.h"
#include "stbox/tsgx/channel/dh_session_initiator.h"
#include "stbox/tsgx/log.h"
#include "ypc_t/analyzer/data_source.h"
#include "ypc_t/ecommon/package.h"
#include <ff/util/ntobject.h>

namespace ypc {
class raw_data_provider : public data_source_with_dhash {
public:
  inline raw_data_provider(const ::stbox::bytes &hash)
      : data_source_with_dhash(hash) {
    m_actual_data_hash = stbox::eth::keccak256_hash(stbox::bytes("Fidelius"));
    m_data_reach_end = false;
  }

  virtual ~raw_data_provider() {}

  virtual bool process() {
    if (m_data_reach_end) {
      return false;
    }
    if (m_item_index + 1 < m_items.size()) {
      m_item_index++;
      return true;
    } else {
      uint8_t *t_sealed_data;
      uint32_t t_sealed_data_len;
      stbox::stx_status ret = static_cast<stbox::stx_status>(
          stbox::ocall_cast<uint32_t>(next_data_batch)(
              m_expect_data_hash.data(), m_expect_data_hash.size(),
              &t_sealed_data, &t_sealed_data_len));

      if (ret != stbox::stx_status::success) {
        m_data_reach_end = true;
        return false;
      }
      // We need move the sealed data from untrusted memory to trusted memory
      stbox::bytes sealed_data(t_sealed_data_len);
      memcpy(sealed_data.data(), t_sealed_data, t_sealed_data_len);
      //! this memory is allocated in next_sealed_item_data, so we need to
      //! deallocate it
      stbox::ocall_cast<void>(free_data_batch)(t_sealed_data);

      typedef nt<stbox::bytes> ntt;
      try {
        auto pkg = make_package<ntt::batch_data_pkg_t>::from_bytes(sealed_data);

        m_items = pkg.get<ntt::batch_data>();
        if (m_items.size() == 0) {
          m_data_reach_end = true;
          return false;
        }

        for (auto b : m_items) {
          stbox::bytes k = m_actual_data_hash + b;
          m_actual_data_hash = stbox::eth::keccak256_hash(k);
        }

        m_item_index = 0;
        return true;
      } catch (const std::exception &e) {
        LOG(ERROR) << "make_package got: " << e.what();
        m_data_reach_end = true;
        return false;
      }

    }
  }

  virtual data_source_output_t output_value() {
    data_source_output_t ret;
    ret.set<nt<bytes>::data>(m_items[m_item_index]);
    return ret;
  }

  virtual const bytes &data_hash() const { return m_actual_data_hash; }

protected:
  bytes m_actual_data_hash;
  std::vector<stbox::bytes> m_items;
  size_t m_item_index;
};
} // namespace ypc
