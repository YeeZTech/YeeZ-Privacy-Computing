#pragma once
#include <exception>
#include <string>

namespace ypc {
class data_sample_too_large : public std::exception {
public:
  data_sample_too_large(const std::string &plugin, const std::string &extra);
  virtual const char *what() throw();

protected:
  const std::string m_plugin;
  const std::string m_extra;

  std::string m_res;
};

class data_format_too_large : public std::exception {
public:
  data_format_too_large(const std::string &plugin, const std::string &extra);
  virtual const char *what() throw();

protected:
  const std::string m_plugin;
  const std::string m_extra;

  std::string m_res;
};

class file_not_found : public std::exception {
public:
  file_not_found(const std::string &path, const std::string &extra);

  virtual const char *what() throw();

protected:
  const std::string m_path;
  const std::string m_extra;

  std::string m_res;
};

class file_open_failure : public std::exception {
public:
  file_open_failure(const std::string &path, const std::string &extra);

  virtual const char *what() throw();

protected:
  const std::string m_path;
  const std::string m_extra;

  std::string m_res;
};
} // namespace ypc
