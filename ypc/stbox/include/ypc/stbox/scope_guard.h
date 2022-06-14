#pragma once
#include <ff/functionflow/utilities/scope_guard.h>

namespace stbox {
using scope_guard = ff::scope_guard;

template <typename T> class malloc_memory_guard {
public:
  typedef T *ptr_type;
  malloc_memory_guard(ptr_type &d) : m_ptr(d) {}
  ~malloc_memory_guard() {
    if (m_ptr) {
      free(m_ptr);
      m_ptr = NULL;
    }
  }

private:
  ptr_type &m_ptr;
};
} // namespace stbox
