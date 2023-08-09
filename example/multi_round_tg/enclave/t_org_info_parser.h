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

define_nt(input_buf, std::string);
typedef ff::net::ntpackage<0, input_buf> input_buf_t;

class t_org_info_parser {
public:
  t_org_info_parser() {}
  t_org_info_parser(
      std::vector<std::shared_ptr<ypc::data_source_with_dhash>> &source)
      : m_datasources(source){};

  inline stbox::bytes do_parse(const stbox::bytes &param) {
    LOG(INFO) << "do parse";
    if (m_datasources.size() != 1) {
      return stbox::bytes("Only one data source");
    }
    ypc::to_type<stbox::bytes, t_org_info_item_t> converter(
        m_datasources[0].get());
    // param must be serialized ntpackage
    auto pkg = ypc::make_package<input_buf_t>::from_bytes(param);
    int counter = 0;
    hpda::processor::internal::filter_impl<t_org_info_item_t> match(
        &converter, [&](const t_org_info_item_t &v) {
          counter++;
          std::string xydm = v.get<shxydm>();
          if (xydm == pkg.get<input_buf>()) {
            return true;
          }
          return false;
        });

    hpda::output::internal::memory_output_impl<t_org_info_item_t> mo(&match);
    mo.get_engine()->run();
    LOG(INFO) << "do parse done";

    typedef ypc::nt<stbox::bytes> ntb;
    ntb::batch_data_pkg_t batch_pkg;
    std::vector<stbox::bytes> batch;
    typedef
        typename ypc::cast_obj_to_package<t_org_info_item_t>::type package_t;
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
