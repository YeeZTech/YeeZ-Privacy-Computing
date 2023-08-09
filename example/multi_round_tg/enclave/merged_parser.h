#include "user_type.h"
#include "ypc/core_t/analyzer/data_source.h"
#include "ypc/corecommon/data_source.h"
#include "ypc/corecommon/package.h"
#include "ypc/corecommon/to_type.h"
#include "ypc/stbox/ebyte.h"
#include "ypc/stbox/stx_common.h"
#include "ypc/stbox/tsgx/log.h"

#include <hpda/extractor/raw_data.h>
#include <hpda/output/memory_output.h>
#include <hpda/processor/query/filter.h>
#include <hpda/processor/set/intersection.h>
#include <hpda/processor/transform/concat.h>

class merged_parser {
public:
  merged_parser() {}
  merged_parser(
      std::vector<std::shared_ptr<ypc::data_source_with_dhash>> &source)
      : m_datasources(source){};

  inline stbox::bytes do_parse(const stbox::bytes &param) {
    LOG(INFO) << "do parse";
    if (m_datasources.size() != 2) {
      return stbox::bytes("Should include two data sources");
    }

    // set intersection
    hpda::processor::internal::ordered_intersection_impl<merged_item_t,
                                                         std::string>
        oi;
    ypc::to_type<stbox::bytes, t_org_info_item_t> conv_a(
        m_datasources[0].get());
    oi.add_upper_stream<::shxydm>(&conv_a);
    LOG(INFO) << "convert to t_org_info_item_t";
    ypc::to_type<stbox::bytes, t_tax_item_t> conv_b(m_datasources[1].get());
    oi.add_upper_stream<::shxydm>(&conv_b);
    LOG(INFO) << "convert to t_tax_item";

    hpda::output::internal::memory_output_impl<merged_item_t> mo(&oi);
    mo.get_engine()->run();
    LOG(INFO) << "do parse done";

    typedef ypc::nt<stbox::bytes> ntb;
    ntb::batch_data_pkg_t batch_pkg;
    std::vector<stbox::bytes> batch;
    typedef typename ypc::cast_obj_to_package<merged_item_t>::type package_t;
    for (auto &it : mo.values()) {
      package_t p = it;
      stbox::bytes b = ypc::make_bytes<stbox::bytes>::for_package(p);
      batch.push_back(b);
    }
    batch_pkg.set<ntb::batch_data>(batch);
    return ypc::make_bytes<stbox::bytes>::for_package(batch_pkg);
  }

protected:
  std::vector<std::shared_ptr<ypc::data_source_with_dhash>> m_datasources;
};
