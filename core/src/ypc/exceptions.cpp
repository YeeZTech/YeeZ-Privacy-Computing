#include "ypc/exceptions.h"
#include <sstream>

namespace ypc {
data_sample_too_large::data_sample_too_large(const std::string &plugin,
                                             const std::string &extra)
    : m_plugin(plugin), m_extra(extra) {}

const char *data_sample_too_large::what() throw() {
  if (m_res.size() > 0) {
    return m_res.c_str();
  }
  std::stringstream ss;
  ss << "Data sample too large: " << m_plugin << ", with " << m_extra << " .";
  m_res = ss.str();
  return m_res.c_str();
}

data_format_too_large::data_format_too_large(const std::string &plugin,
                                             const std::string &extra)
    : m_plugin(plugin), m_extra(extra) {}

const char *data_format_too_large::what() throw() {
  if (m_res.size() > 0) {
    return m_res.c_str();
  }
  std::stringstream ss;
  ss << "Data format too large: " << m_plugin << ", with " << m_extra << " .";
  m_res = ss.str();
  return m_res.c_str();
}

file_not_found::file_not_found(const std::string &path,
                               const std::string &extra)
    : m_path(path), m_extra(extra) {}

const char *file_not_found::what() throw() {
  if (m_res.size() > 0) {
    return m_res.c_str();
  }
  std::stringstream ss;
  ss << "File " << m_path << " not found. (" << m_extra << ")";
  m_res = ss.str();
  return m_res.c_str();
}

file_open_failure::file_open_failure(const std::string &path,
                                     const std::string &extra)
    : m_path(path), m_extra(extra) {}

const char *file_open_failure::what() throw() {
  if (m_res.size() > 0) {
    return m_res.c_str();
  }
  std::stringstream ss;
  ss << "Cannot open file " << m_path << ". (" << m_extra << ")";
  m_res = ss.str();
  return m_res.c_str();
}
} // namespace ypc
