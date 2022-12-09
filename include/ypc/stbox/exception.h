#pragma once
#include "ypc/stbox/stx_status.h"
#include <exception>
#include <sgx_error.h>

namespace stbox {
class st_error {
public:
  explicit st_error(sgx_status_t s);

  virtual const char *what();

protected:
  sgx_status_t m_status;
  std::string m_message;
};
} // namespace stbox
