#pragma once
#include <exception>
#include <sgx_error.h>
#include <stbox/stx_status.h>

namespace stbox {
class st_error {
public:
  st_error(sgx_status_t s);

  virtual const char *what();

protected:
  sgx_status_t m_status;
  std::string m_message;
};
} // namespace stbox
