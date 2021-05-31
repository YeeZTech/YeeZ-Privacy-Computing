#include "stbox/exception.h"
namespace stbox {
st_error::st_error(sgx_status_t s) : m_status(s) {}
const char *st_error::what() {
  if (m_message.empty()) {
    m_message = std::string(status_string(m_status));
  }
  return m_message.c_str();
}
} // namespace stbox
