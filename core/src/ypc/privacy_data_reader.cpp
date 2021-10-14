#include "ypc/privacy_data_reader.h"
#include "common/limits.h"
#include "ypc/exceptions.h"
#include "ypc/filesystem.h"
#include <boost/format.hpp>
#include <iostream>

namespace ypc {
component_load_failure::component_load_failure(const std::string &name)
    : std::exception(), m_name(name) {
  m_res = (boost::format("failed to load component %1%") % m_name).str();
}

const char *component_load_failure::what() const throw() {
  return m_res.c_str();
}

component_sym_failure::component_sym_failure(const std::string &name,
                                             const std::string &sym)
    : std::exception(), m_name(name), m_sym(sym) {

  m_res =
      (boost::format("failed to load sym %1% component %2%") % m_name % m_sym)
          .str();
}

const char *component_sym_failure::what() const throw() {
  return m_res.c_str();
}

privacy_data_reader::privacy_data_reader(const std::string &plugin_path,
                                         const std::string &extra_param)
    : m_plugin_path(complete_path(plugin_path)), m_extra_param(extra_param) {
  m_lib_handle = dlopen(m_plugin_path.c_str(), RTLD_LAZY);
  if (!m_lib_handle) {
    LOG(ERROR) << "failed to open " << m_plugin_path
               << " with error: " << dlerror();
    throw std::runtime_error("failed to call dlopen");
  }

  m_create_item_reader =
      get_func_with_name<create_item_reader_func_t>("create_item_reader");
  m_reset_for_read =
      get_func_with_name<reset_for_read_func_t>("reset_for_read");
  m_read_item_data =
      get_func_with_name<read_item_data_func_t>("read_item_data");
  m_close_item_reader =
      get_func_with_name<close_item_reader_func_t>("close_item_reader");

  m_get_item_number =
      get_func_with_name<get_item_number_func_t>("get_item_number");
  m_get_sample_data =
      get_opt_func_with_name<get_sample_data_func_t>("get_sample_data");
  m_get_data_format =
      get_opt_func_with_name<get_data_format_func_t>("get_data_format");

  m_handle = m_create_item_reader(m_extra_param.c_str(), m_extra_param.size());
  if (!m_handle) {
    throw std::runtime_error("failed to create item reader");
  }
}
privacy_data_reader::~privacy_data_reader() {
  if (m_handle) {
    m_close_item_reader(m_handle);
    m_handle = nullptr;
  }
}

void privacy_data_reader::reset_for_read() { m_reset_for_read(m_handle); }

uint64_t privacy_data_reader::get_item_number() {
  return m_get_item_number(m_handle);
}

bytes privacy_data_reader::get_sample_data() {
  if (!m_get_sample_data) {
    return bytes();
  }
  int len;
  m_get_sample_data(m_handle, NULL, &len);
  if (len > ::ypc::utc::max_sample_size) {
    throw data_sample_too_large(m_plugin_path, m_extra_param);
  }
  bytes ret(len);
  m_get_sample_data(m_handle, (char *)&ret[0], &len);

  return ret;
}

std::string privacy_data_reader::get_data_format() {
  if (!m_get_data_format) {
    return std::string();
  }
  int len;
  m_get_data_format(m_handle, NULL, &len);
  if (len > ::ypc::utc::max_data_format_size) {
    throw data_format_too_large(m_plugin_path, m_extra_param);
  }
  std::string ret(len, 0);
  m_get_data_format(m_handle, &ret[0], &len);
  return ret;
}

bytes privacy_data_reader::read_item_data() {
  // We use static buf here to optimize memory usage.
  char buf[::ypc::utc::max_item_size] = {0};

  int len = ::ypc::utc::max_item_size;
  auto status = m_read_item_data(m_handle, buf, &len);
  if (status != 0) {
    return bytes();
  }
  return bytes(buf, len);
}
} // namespace ypc
