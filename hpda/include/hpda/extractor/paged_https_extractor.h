#pragma once
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <ff/util/ntobject.h>
#include <hpda/common/common.h>
#include <hpda/common/stream_policy.h>
#include <hpda/extractor/extractor_base.h>

namespace hpda {
namespace extractor {
namespace internal {
class paged_https_extractor_impl : public extractor_base<std::string> {
public:
  typedef std::function<std::string(const std::string &prefix, int next_index,
                                    int page_limit)>
      next_page_request_t;
  typedef std::function<int(const boost::property_tree::ptree &tree)>
      start_index_picker_t;
  typedef std::function<int(const boost::property_tree::ptree &tree)>
      end_index_picker_t;

  paged_https_extractor_impl(const std::string &domain, const std::string &path,
                             int page_limit, const next_page_request_t &func,
                             const start_index_picker_t &start_index_picker,
                             const end_index_picker_t &end_index_picker);

  virtual ~paged_https_extractor_impl();
  virtual bool process();

  virtual std::string output_value();

protected:
  std::string request(const std::string &url);

protected:
  const std::string m_domain;
  const std::string m_path;
  int32_t m_index;
  int32_t m_page_limit;
  bool m_reach_end;
  std::string m_response;
  next_page_request_t m_next_page_request;
  start_index_picker_t m_start_index_picker;
  end_index_picker_t m_end_index_picker;
};

} // namespace internal

using paged_https_extractor = internal::paged_https_extractor_impl;

} // namespace extractor
} // namespace hpda
