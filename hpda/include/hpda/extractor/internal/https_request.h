#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <hpda/common/common.h>

namespace hpda {
namespace extractor {
namespace internal {

class https_request {
public:
  https_request(const std::string &domain, const std::string &path);

  const std::string &result() const;

protected:
  mutable boost::asio::ssl::context m_ctx;
  mutable boost::asio::io_service m_io_service;
  std::string m_domain;
  std::string m_path;
  mutable std::string m_result;
};
} // namespace internal
} // namespace extractor
} // namespace hpda
