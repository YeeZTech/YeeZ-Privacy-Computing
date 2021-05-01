#pragma once
#include <atomic>
#include <memory>

namespace ypc {
template <typename T> class ref {
public:
  ref() : m_data(nullptr), m_len(0) {}
  ref(T *data, size_t len)
      : m_data(data),
        m_len(len){
    m_holder = std::shared_ptr<T>(m_data, ::ypc::ref<T>::delete_wild_pointer);
  };

  inline size_t &len() { return m_len; }
  inline const size_t &len() const { return m_len; }

  inline size_t &size() { return m_len; }
  inline const size_t &size() const { return m_len; }

  inline T *data() { return m_data; }
  inline const T *data() const { return m_data; }

  static void delete_wild_pointer(T *p) { free(p); }

private:
  T *m_data;
  size_t m_len;
  std::shared_ptr<T> m_holder;
};

typedef ref<uint8_t> bref;
} // namespace ypc

