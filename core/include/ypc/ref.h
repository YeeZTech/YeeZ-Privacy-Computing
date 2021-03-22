#pragma once
#include <atomic>
#include <memory>

namespace ypc {
template <typename T> class ref {
public:
  template <typename TK> struct wild_pointer_deleter {
    void operator()(TK const *p) { free(p); }
  };

  ref() : m_data(nullptr), m_len(0) {}
  ref(T *data, size_t len)
      : m_data(data),
        m_len(len){
    m_holder = std::shared_ptr<T>(m_data, ::ypc::ref<T>::delete_wild_pointer);
  };

  inline size_t &len() { return m_len; }
  inline const size_t &len() const { return m_len; }
  inline T *data() { return m_data; }
  inline const T *data() const { return m_data; }

  static void delete_wild_pointer(T *p) { free(p); }

private:
  T *m_data;
  size_t m_len;
  std::shared_ptr<T> m_holder;
};

typedef ref<uint8_t> bref;

std::string to_hex(const bref &s);

} // namespace ypc

