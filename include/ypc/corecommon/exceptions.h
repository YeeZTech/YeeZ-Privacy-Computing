#pragma once
#include <exception>
#include <string>

namespace ypc {
class data_sample_too_large : public std::exception {
public:
  inline data_sample_too_large(const std::string &plugin,
                               const std::string &extra) {
    m_res = std::string("Data sample too large: ") + plugin + ", with " +
            extra + ".";
  }
  inline virtual const char *what() final { return m_res.c_str(); }

protected:
  std::string m_res;
};

class data_format_too_large : public std::exception {
public:
  inline data_format_too_large(const std::string &plugin,
                               const std::string &extra) {
    m_res = std::string("Data format too large: ") + plugin + ", with " +
            extra + ".";
  }
  inline virtual const char *what() final { return m_res.c_str(); }

protected:
  std::string m_res;
};

class file_not_found : public std::exception {
public:
  inline file_not_found(const std::string &path, const std::string &extra) {
    m_res = std::string("File ") + path + " not found. (" + extra + ")";
  }

  inline virtual const char *what() final { return m_res.c_str(); };

protected:
  std::string m_res;
};

class file_open_failure : public std::exception {
public:
  inline file_open_failure(const std::string &path, const std::string &extra) {
    m_res = std::string("Cannot open file ") + path + ". (" + extra + ")";
  }

  inline virtual const char *what() final { return m_res.c_str(); };

protected:
  std::string m_res;
};
} // namespace ypc
