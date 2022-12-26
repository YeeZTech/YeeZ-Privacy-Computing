#include "ypc/core_t/util/cxxfile.h"
#include "eparser_t.h"
#include "ypc/stbox/tsgx/log.h"
#include "ypc/stbox/tsgx/ocall.h"

namespace ypc {
cxxfile::cxxfile() : m_stream_id(0) {}
void cxxfile::open(const char *filename, ypc::ios_base::openmode mode) {
  uint32_t status = stbox::ocall_cast<uint32_t>(fopen_ocall)(
      filename, strlen(filename), mode);
  if (0 == status) {
    throw std::runtime_error("fopen failed");
  }
  m_stream_id = status;
}
bool cxxfile::is_open() const { return m_stream_id != 0; }

#define CHECK_OPEN                                                             \
  if (!is_open()) {                                                            \
    throw std::runtime_error("file not open");                                 \
  }

void cxxfile::close() {
  CHECK_OPEN;
  stbox::ocall_cast<void>(fclose_ocall)(m_stream_id);
  m_stream_id = 0;
}

cxxfile &cxxfile::seekg(const fpos &pos, ios_base::seekdir dir) {
  CHECK_OPEN;

  stbox::ocall_cast<void>(fseek_ocall)(m_stream_id, pos.data(), dir);
  return *this;
}
fpos cxxfile::tellg() {
  CHECK_OPEN;
  fpos ret;
  stbox::ocall_cast<void>(ftell_ocall)(m_stream_id, ret.data());
  return ret;
}
cxxfile &cxxfile::flush() {
  CHECK_OPEN;
  stbox::ocall_cast<void>(fflush_ocall)(m_stream_id);
  return *this;
}

cxxfile &cxxfile::read(uint8_t *s, size_t size) {
  CHECK_OPEN;
  stbox::ocall_cast<void>(fread_ocall)(s, size, m_stream_id);
  return *this;
}
cxxfile &cxxfile::write(const uint8_t *s, size_t size) {
  CHECK_OPEN;
  stbox::ocall_cast<void>(fwrite_ocall)(s, size, m_stream_id);
  return *this;
}
bool cxxfile::good() const {
  if (!is_open()) {
    return false;
  }
  return stbox::ocall_cast<uint8_t>(fgood_ocall)(m_stream_id) != 0;
}
bool cxxfile::eof() const {
  if (!is_open()) {
    return true;
  }
  return stbox::ocall_cast<uint8_t>(feof_ocall)(m_stream_id) != 0;
}
bool cxxfile::fail() const {
  if (!is_open()) {
    return true;
  }
  return stbox::ocall_cast<uint8_t>(ffail_ocall)(m_stream_id) != 0;
}
bool cxxfile::bad() const {
  if (!is_open()) {
    return true;
  }
  return stbox::ocall_cast<uint8_t>(fbad_ocall)(m_stream_id) != 0;
}
} // namespace ypc
