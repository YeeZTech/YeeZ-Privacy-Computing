#pragma once
#include "ypc/byte.h"
#include "ypc/status.h"

class param_source {
public:
  virtual uint32_t read_from_source() = 0;

  inline param_source &copy_from(param_source &s) {
    if (&s == this) {
      return *this;
    }
    s.read_from_source();

    m_eskey = s.eskey();
    m_input = s.input();
    m_epkey = s.epkey();
    m_ehash = s.ehash();
    m_vpkey = s.vpkey();
    m_sig = s.sig();
    return *this;
  }
  inline param_source &operator=(param_source &s) { return copy_from(s); }

  inline const ypc::bytes &eskey() const { return m_eskey; }

  inline const ypc::bytes &input() const { return m_input; }
  inline const ypc::bytes &epkey() const { return m_epkey; }
  inline const ypc::bytes &ehash() const { return m_ehash; }
  inline const ypc::bytes &vpkey() const { return m_vpkey; }
  inline const ypc::bytes &sig() const { return m_sig; }

  inline ypc::bytes &input() { return m_input; }
  inline ypc::bytes &epkey() { return m_epkey; }
  inline ypc::bytes &ehash() { return m_ehash; }
  inline ypc::bytes &vpkey() { return m_vpkey; }
  inline ypc::bytes &sig() { return m_sig; }

protected:
  ypc::bytes m_eskey;
  ypc::bytes m_input;
  ypc::bytes m_epkey;
  ypc::bytes m_ehash;
  ypc::bytes m_vpkey;
  ypc::bytes m_sig;
};
