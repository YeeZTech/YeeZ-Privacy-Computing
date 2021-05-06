#pragma once
#include "stbox/ebyte.h"
#include "stbox/stx_common.h"
#include "stbox/stx_status.h"
#include <string>

namespace stbox {

typedef enum log_rank {
  INFO,
  WARNING,
  ERROR,
  FATAL

} log_rank_t;

class Logger {
public:
  inline Logger(log_rank_t log_rank) : m_log_rank(log_rank){};

  ~Logger();
  Logger &start(const std::string &file, uint32_t line,
                const std::string &function);
  template <typename T>
  auto operator<<(const T &t) ->
      typename std::enable_if<std::is_arithmetic<T>::value, Logger &>::type {
    m_ss = m_ss + std::to_string(t);
    return *this;
  }
  Logger &operator<<(const std::string &t);
  Logger &operator<<(const bytes &t);
  Logger &operator<<(stx_status t);
  Logger &operator<<(sgx_status_t t);

private:
  std::string m_ss;
  log_rank_t m_log_rank;
};

} // namespace stbox

#define LOG(log_rank)                                                          \
  ::stbox::Logger(stbox::log_rank).start(__FILE__, __LINE__, __FUNCTION__)
