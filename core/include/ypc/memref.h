#pragma once
#include <cassert>
#include <memory>

namespace ypc {
class memref {
public:
  inline memref() : m_data(nullptr), m_len(0){};
  inline size_t &len() { return m_len; }
  inline const size_t &len() const { return m_len; }
  inline char *data() { return m_data; }
  inline const char *data() const { return m_data; }

  inline void alloc(size_t s) {
    assert(!m_data);
    m_data = new char[s];
    m_len = s;
  }
  inline void dealloc() {
    if (m_data) {
      delete[] m_data;
      m_data = nullptr;
      m_len = 0;
    }
  }

private:
  char *m_data;
  size_t m_len;
};
} // namespace ypc
