#pragma once
#include <functional>
#include <sgx_eid.h>
#include <sgx_error.h>
#include <stbox/exception.h>

namespace stbox {

template <typename FT> class ocall {};
template <typename... ARGS> class ocall<void(ARGS...)> {
public:
  typedef std::function<sgx_status_t(ARGS...)> func_type;
  ocall() : m_func() {}
  ocall(const func_type &func) : m_func(func){};
  virtual ~ocall() {}

  void operator()(ARGS... args) {
    sgx_status_t ret = m_func(args...);
    if (ret != SGX_SUCCESS) {
      throw st_error(ret);
    }
  }

protected:
  func_type m_func;
};

template <typename RetType, typename... ARGS> class ocall<RetType(ARGS...)> {
public:
  typedef std::function<sgx_status_t(RetType *, ARGS...)> func_type;
  ocall() : m_func() {}
  ocall(const func_type &func) : m_func(func){};
  virtual ~ocall() {}

  RetType operator()(ARGS... args) {
    RetType retval;
    sgx_status_t ret = m_func(&retval, args...);
    if (ret != SGX_SUCCESS) {
      throw st_error(ret);
    }
    return retval;
  }
protected:
  func_type m_func;
};

template <typename RetType, typename FT,
          bool flag = std::is_same<RetType, void>::value>
struct get_ocall_type {};

template <typename... ARGS>
struct get_ocall_type<void, sgx_status_t(ARGS...), true> {
  typedef ocall<void(ARGS...)> type;
};

template <typename RetType, typename... ARGS>
struct get_ocall_type<RetType, sgx_status_t(RetType *, ARGS...), false> {
  typedef ocall<RetType(ARGS...)> type;
};
template <typename RetType, typename KT, bool flag, typename... ARGS>
struct get_ocall_type<RetType, KT(ARGS...), flag> {
  static_assert(std::is_same<KT, sgx_status_t>::value,
                "not a SGX ocall function");
};
template <typename RetType, typename KT, typename... ARGS>
struct get_ocall_type<RetType, sgx_status_t(KT, ARGS...), false> {
  static_assert(std::is_same<RetType *, KT>::value,
                "Return Type doesn't not match with the first param type");
};

template <typename RetType, typename FT>
auto ocall_cast(const FT &f) -> typename get_ocall_type<RetType, FT>::type {
  return typename get_ocall_type<RetType, FT>::type(f);
}
} // namespace stbox
