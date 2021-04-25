#include <cstdarg>
#include <glog/logging.h>
#include <iostream>
#include <string.h>

namespace stbox {
int printf(const char *fmt, ...) {
  char buf[BUFSIZ] = {'\0'};
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, BUFSIZ, fmt, ap);
  va_end(ap);
  std::cout << buf << std::flush;
  return (int)strnlen(buf, BUFSIZ - 1) + 1;
}
} // namespace stbox

extern "C" {
void ocall_print_string(const char *buf);
void ocall_log_string(uint32_t rank, const char *buf);
}
namespace stbox {
typedef enum log_rank {
  INFO,
  WARNING,
  ERROR,
  FATAL

} log_rank_t;
}
void ocall_print_string(const char *buf) { std::cout << buf << std::flush; }
void ocall_log_string(uint32_t rank, const char *buf) {
  switch (rank) {
  case stbox::log_rank::INFO:
    LOG(INFO) << buf;
    google::FlushLogFiles(google::INFO);
    break;
  case stbox::log_rank::WARNING:
    LOG(WARNING) << buf;
    google::FlushLogFiles(google::WARNING);
    break;
  case stbox::log_rank::ERROR:
    LOG(ERROR) << buf;
    google::FlushLogFiles(google::ERROR);
    break;
  case stbox::log_rank::FATAL:
    LOG(FATAL) << buf;
    google::FlushLogFiles(google::FATAL);
    break;
  default:
    LOG(ERROR) << "unexpected rank: " << buf;
  }
}
