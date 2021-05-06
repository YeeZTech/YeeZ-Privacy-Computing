#pragma once
#include <functional>
#include <sgx_eid.h>
#include <sgx_error.h>
#include <sgx_urts.h>
#include <stbox/stx_status.h>
#include <string>

namespace stbox {
namespace internal {
class sgx_module_base {
public:
  virtual ~sgx_module_base();

  inline sgx_enclave_id_t sgx_eid() const { return m_sgx_eid; }

  template <typename RetType, typename Func, typename... ARGS>
  RetType no_check_ecall(const Func &f, ARGS... args) {
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    for (;;) {
      RetType retvalue;
      ret = f(m_sgx_eid, &retvalue, args...);
      //! Power transition occured
      if (ret == SGX_ERROR_ENCLAVE_LOST) {
        if (m_sgx_eid != 0) {
          sgx_destroy_enclave(m_sgx_eid);
        }
        ret = sgx_create_enclave(m_mod_path.c_str(), SGX_DEBUG_FLAG, NULL, NULL,
                                 &m_sgx_eid, NULL);
        if (ret != SGX_SUCCESS) {
          throw std::runtime_error(status_string(ret));
        }
        continue;
      }
      if (ret != SGX_SUCCESS) {
        throw std::runtime_error(status_string(ret));
      }
      return retvalue;
    }
  }

protected:
  std::string m_mod_path;
  sgx_enclave_id_t m_sgx_eid;
}; // end sgx_module_base

struct function_object_base {
  virtual uint32_t call(sgx_module_base &m) = 0;
};
template <typename... ARGS> struct function_object;
#include "sgx_module_function_object.ipp"

struct buffer_length_traits {
  // typedef sgx_status_t (*func_t)(sgx_enclave_id_t, uint32_t *);

  template <typename FuncType, typename... ARGS>
  inline buffer_length_traits(uint32_t *len, uint8_t **mem, FuncType &&f,
                              ARGS... args)
      : m_len(len), m_mem(mem),
        m_func(
            new function_object<typename std::remove_reference<ARGS>::type...>(
                f, args...)) {}

  template <typename FuncType, typename... ARGS>
  inline buffer_length_traits(uint8_t **mem, uint32_t *len, FuncType &&f,
                              ARGS... args)
      : m_len(len), m_mem(mem),
        m_func(
            new function_object<typename std::remove_reference<ARGS>::type...>(
                f, args...)) {}

  inline ~buffer_length_traits() {
    delete m_func;
  }

  void call(sgx_module_base &m);
  uint32_t *m_len;
  uint8_t **m_mem;
  function_object_base *m_func;
};

struct xmem {
  inline xmem(buffer_length_traits &m) : mm(m) {}
  buffer_length_traits &mm;
};

struct xlen {
  inline xlen(buffer_length_traits &m) : mm(m) {}
  buffer_length_traits &mm;
};

template <typename T> struct param_extractor {
  param_extractor(const T &t) : value(t) {}
  typedef T type;
  T value;
};

template <> struct param_extractor<xmem> {
  param_extractor(const xmem &t) : value(*t.mm.m_mem) {}
  typedef uint8_t *type;
  uint8_t *value;
};
template <> struct param_extractor<xlen> {
  param_extractor(const xlen &t) : value(*t.mm.m_len) {}
  typedef uint32_t type;
  uint32_t value;
};

template <typename T> auto et(const T &t) -> typename param_extractor<T>::type {
  return param_extractor<T>(t).value;
}

template <typename T> struct mem_init_helper {
  static void call(sgx_module_base &m, T &t) {}
};

template <> struct mem_init_helper<xlen> {
  static void call(sgx_module_base &m, xlen &t) {
    t.mm.call(m);
  }
};

template <typename T> int mem_init(sgx_module_base &m, T &t) {
  mem_init_helper<T>::call(m, t);
  return 0;
};

} // namespace internal

class sgx_module : public internal::sgx_module_base {
public:
  sgx_module(const char *mod_path);

  virtual ~sgx_module();

  template <typename RetType, typename Func, typename... ARGS>
  RetType ecall(const Func &f, ARGS... args) {
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    placeholder_func(internal::mem_init(*this, args)...);

    for (;;) {
      RetType retvalue;
      ret = f(m_sgx_eid, &retvalue, internal::et(args)...);
      //! Power transition occured
      if (ret == SGX_ERROR_ENCLAVE_LOST) {
        if (m_sgx_eid != 0) {
          sgx_destroy_enclave(m_sgx_eid);
        }
        ret = sgx_create_enclave(m_mod_path.c_str(), SGX_DEBUG_FLAG, NULL, NULL,
                                 &m_sgx_eid, NULL);
        if (ret != SGX_SUCCESS) {
          throw std::runtime_error(status_string(ret));
        }
        continue;
      }
      if (ret != SGX_SUCCESS) {
        throw std::runtime_error(status_string(ret));
      }
      return retvalue;
    }
  }

private:
  template <typename... ARGS> void placeholder_func(ARGS... args) {}
};

using buffer_length_t = internal::buffer_length_traits;
using xmem = internal::xmem;
using xlen = internal::xlen;
} // namespace stbox
