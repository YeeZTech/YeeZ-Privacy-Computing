#pragma once
#include "ypc/core/byte.h"
#include <dlfcn.h>
#include <exception>
#include <string>

namespace ypc {
class component_load_failure : public std::exception {
public:
  explicit component_load_failure(const std::string &name);
  virtual const char *what() const throw();

protected:
  const std::string m_name;
  std::string m_res;
};

class component_sym_failure : public std::exception {
public:
  component_sym_failure(const std::string &name, const std::string &sym);
  virtual const char *what() const throw();

protected:
  const std::string m_name;
  const std::string m_sym;
  std::string m_res;
};

class privacy_data_reader {
public:
  privacy_data_reader(const std::string &plugin_path,
                      const std::string &extra_param);
  virtual ~privacy_data_reader();

  privacy_data_reader(const privacy_data_reader &) = delete;
  privacy_data_reader(privacy_data_reader &&) = delete;
  privacy_data_reader &operator=(privacy_data_reader &&) = delete;
  privacy_data_reader &operator=(const privacy_data_reader &) = delete;

  void reset_for_read();
  bytes read_item_data();
  uint64_t get_item_number();
  bytes get_sample_data();
  std::string get_data_format();

protected:
  template <typename T> T get_func_with_name(const std::string &name) {
    T r = (T)dlsym(m_lib_handle, name.c_str());
    if (!r) {
      throw component_sym_failure(m_plugin_path + "::" + name, dlerror());
    }
    return r;
  }

  template <typename T> T get_opt_func_with_name(const std::string &name) {
    T r = (T)dlsym(m_lib_handle, name.c_str());
    return r;
  }

  // NOLINTBEGIN(modernize-use-using)
  typedef void *(*create_item_reader_func_t)(const char *, int); // NOLINT
  typedef int (*reset_for_read_func_t)(void *);                  // NOLINT
  typedef int (*read_item_data_func_t)(void *, char *, int *);   // NOLINT
  typedef int (*close_item_reader_func_t)(void *);               // NOLINT
  typedef uint64_t (*get_item_number_func_t)(void *);            // NOLINT
  typedef int (*get_sample_data_func_t)(void *, char *, int *);  // NOLINT
  typedef int (*get_data_format_func_t)(void *, char *, int *);  // NOLINT
  // NOLINTEND(modernize-use-using)

  const std::string m_plugin_path;
  const std::string m_extra_param;

  void *m_handle;

  void *m_lib_handle;
  create_item_reader_func_t m_create_item_reader;
  reset_for_read_func_t m_reset_for_read;
  read_item_data_func_t m_read_item_data;
  close_item_reader_func_t m_close_item_reader;

  get_item_number_func_t m_get_item_number;
  get_sample_data_func_t m_get_sample_data;
  get_data_format_func_t m_get_data_format;
};

} // namespace ypc
