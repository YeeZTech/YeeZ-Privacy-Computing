#pragma once
#include <functional>
#include <memory>
#include <mutex>

namespace ypc {

template <typename T> class singleton {
public:
  static T &instance() {
    std::call_once(s_init_once, std::bind(singleton<T>::init));
    if (!s_pInstance) {
      throw std::runtime_error("already deallocated!");
    }
    return *s_pInstance;
  }

  void release() {
    std::call_once(s_dealloc_once, std::bind(singleton<T>::dealloc));
  }

protected:
  singleton() = default;

private:
  static void init() { s_pInstance = std::shared_ptr<T>(new T()); }
  static void dealloc() { s_pInstance.reset(); }

protected:
  static std::shared_ptr<T> s_pInstance;
  static std::once_flag s_init_once;
  static std::once_flag s_dealloc_once;
};
template <typename T> std::shared_ptr<T> singleton<T>::s_pInstance;
template <typename T> std::once_flag singleton<T>::s_init_once;
template <typename T> std::once_flag singleton<T>::s_dealloc_once;

template <typename T> class singleton_guard {
public:
  singleton_guard() = default;
  singleton_guard(const singleton_guard &) = delete;
  singleton_guard &operator=(const singleton_guard &) = delete;
  singleton_guard(singleton_guard &&) = delete;
  singleton_guard &operator=(singleton_guard &&) = delete;

  ~singleton_guard() { singleton<T>::instance().release(); }
}; // end class singleton_guard
} // namespace ypc
