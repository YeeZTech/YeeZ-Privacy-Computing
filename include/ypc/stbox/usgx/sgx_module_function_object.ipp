template <> struct function_object<> : public function_object_base{
  typedef sgx_status_t (*func_t)(sgx_enclave_id_t, uint32_t *);

  function_object(func_t f) : m_func(f){};

  virtual uint32_t call(sgx_module_base &m) {
    return m.no_check_ecall<uint32_t>(m_func);
  }

protected:
  func_t m_func;
};

template <typename T> struct function_object<T> : public function_object_base{
  typedef sgx_status_t (*func_t)(sgx_enclave_id_t, uint32_t *, T);

  function_object(func_t f, const T &arg1) : m_func(f), m_arg1(arg1){};

  virtual uint32_t call(sgx_module_base &m) {
    return m.no_check_ecall<uint32_t>(m_func, m_arg1);
  }

protected:
  func_t m_func;
  T m_arg1;
};

template <typename T1, typename T2> struct function_object<T1, T2> : public function_object_base{
  typedef sgx_status_t (*func_t)(sgx_enclave_id_t, uint32_t *, T1, T2);

  function_object(func_t f, const T1 & arg1, const T2 & arg2)
    : m_func(f), m_arg1(arg1), m_arg2(arg2){};

  virtual uint32_t call(sgx_module_base &m) {
    return m.no_check_ecall<uint32_t>(m_func, m_arg1, m_arg2);
  }

protected:
  func_t m_func;
  T1 m_arg1;
  T2 m_arg2;
};

template <typename T1, typename T2, typename T3> struct function_object<T1, T2, T3> : public function_object_base{
  typedef sgx_status_t (*func_t)(sgx_enclave_id_t, uint32_t *, T1, T2, T3);

  function_object(func_t f, const T1 & arg1, const T2 & arg2, const T3 & arg3)
    : m_func(f), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3){};

  virtual uint32_t call(sgx_module_base &m) {
    return m.no_check_ecall<uint32_t>(m_func, m_arg1, m_arg2, m_arg3);
  }

protected:
  func_t m_func;
  T1 m_arg1;
  T2 m_arg2;
  T3 m_arg3;
};
