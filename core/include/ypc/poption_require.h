#pragma once
#include <boost/program_options.hpp>
#include <iostream>
namespace ypc {

namespace internal {
class opt_base {
public:
  virtual bool check(const boost::program_options::variables_map &vm,
                     bool exit_if_fail) const = 0;
  virtual std::string to_string() const = 0;
};
class opt : public opt_base {
public:
  template <typename T> opt(T &&_s) : m_value(_s) {}

  virtual bool check(const boost::program_options::variables_map &vm,
                     bool exit_if_fail) const;
  virtual std::string to_string() const { return m_value; }

protected:
  std::string m_value;
};

template <typename O1, typename O2> class and_opt : public opt_base {
public:
  and_opt(const O1 &o1, const O2 &o2) : m_o1(o1), m_o2(o2) {}
  virtual bool check(const boost::program_options::variables_map &vm,
                     bool exit_if_fail) const {
    bool v1 = m_o1.check(vm, exit_if_fail);
    bool v2 = m_o2.check(vm, exit_if_fail);
    if (exit_if_fail && (!v1 || !v2)) {
      std::cerr << "missing '" << m_o1.to_string() << "' and '"
                << m_o2.to_string() << "'" << std::endl;
      exit(-1);
    }
    return v1 && v2;
  }

  virtual std::string to_string() const {
    return m_o1.to_string() + " && " + m_o2.to_string();
  }

protected:
  O1 m_o1;
  O2 m_o2;
};
template <typename O1, typename O2> class or_opt : public opt_base {
public:
  or_opt(const O1 &o1, const O2 &o2) : m_o1(o1), m_o2(o2) {}
  virtual bool check(const boost::program_options::variables_map &vm,
                     bool exit_if_fail) const {
    bool v1 = m_o1.check(vm, false);
    bool v2 = m_o2.check(vm, false);
    if (exit_if_fail && !v1 && !v2) {
      std::cerr << "missing '" << m_o1.to_string() << "' or '"
                << m_o2.to_string() << "'." << std::endl;
      exit(-1);
    }
    return v1 || v2;
  }

  virtual std::string to_string() const {
    return m_o1.to_string() + " || " + m_o2.to_string();
  }

protected:
  O1 m_o1;
  O2 m_o2;
};

} // namespace internal


class po {
public:
  inline explicit po(const boost::program_options::variables_map &vm)
      : m_vm(vm){};

  template <typename OT>
  auto require(const OT &o) -> typename std::enable_if<
      std::is_base_of<internal::opt_base,
                      typename std::remove_reference<OT>::type>::value,
      void>::type {
    o.check(m_vm, true);
  };
  template <typename OT>
  auto require(const OT &o) -> typename std::enable_if<
      !std::is_base_of<internal::opt_base,
                       typename std::remove_reference<OT>::type>::value,
      void>::type {
    require(internal::opt(o));
  }

protected:
  const boost::program_options::variables_map &m_vm;
};
using opt = internal::opt;

} // namespace ypc

template <typename O1, typename O2>
auto operator&&(O1 &&lhs, O2 &&rhs) -> typename std::enable_if<
    std::is_base_of<ypc::internal::opt_base,
                    typename std::remove_reference<O1>::type>::value &&
        std::is_base_of<ypc::internal::opt_base,
                        typename std::remove_reference<O2>::type>::value,
    ypc::internal::and_opt<O1, O2>>::type {
  return ypc::internal::and_opt<O1, O2>(lhs, rhs);
}

template <typename O1, typename O2>
auto operator||(const O1 &lhs, const O2 &rhs) -> typename std::enable_if<
    std::is_base_of<ypc::internal::opt_base, O1>::value &&
        std::is_base_of<ypc::internal::opt_base, O2>::value,
    ypc::internal::or_opt<O1, O2>>::type {
  return ypc::internal::or_opt<O1, O2>(lhs, rhs);
}
