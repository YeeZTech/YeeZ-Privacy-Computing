#pragma once
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <ff/util/ntobject.h>
#include <glog/logging.h>

namespace ypc {
template <typename CT> struct user_def_name {};

namespace internal {

template <typename T, typename = int> struct has_name : std::false_type {};
template <typename T>
struct has_name<T, decltype((void)T::name, 0)> : std::true_type {};

template <typename CT,
          bool is_user_def = internal::has_name<user_def_name<CT>>::value>
struct name_traits {
  constexpr static const char *name = user_def_name<CT>::name;
};
template <typename CT> struct name_traits<CT, false> {
  constexpr static const char *name = ff::util::internal::nt_traits<CT>::name;
};

template <typename CType,
          typename UnderlyingType =
              typename ff::util::internal::nt_traits<CType>::type>
struct ptree_put_helper {
  template <typename NtObjTy>
  static void write(boost::property_tree::ptree &pt, const NtObjTy &data) {
    pt.put(name_traits<CType>::name, data.template get<CType>());
  }
};
template <typename CType,
          typename UnderlyingType =
              typename ff::util::internal::nt_traits<CType>::type>
struct ptree_get_helper {
  template <typename NtObjTy>
  static void read(const boost::property_tree::ptree &pt, NtObjTy &data) {
    if (pt.find(name_traits<CType>::name) != pt.not_found()) {
      data.template set<CType>(
          pt.get<UnderlyingType>(name_traits<CType>::name));
    } else {
      LOG(WARNING) << "cannot find json item with name: "
                   << name_traits<CType>::name
                   << ", use default value: " << UnderlyingType();
    }
  }
};
template <typename T> struct to_ptree {
  static boost::property_tree::ptree f(const T &value) {
    boost::property_tree::ptree child(std::to_string(value));
    return child;
  }
};

template <typename T> struct from_ptree {
  static T f(const boost::property_tree::ptree &ptree) {
    std::stringstream ss;
    ss << ptree.data();
    T t;
    ss >> t;
    return t;
  }
};

template <typename CType, typename T>
struct ptree_put_helper<CType, std::vector<T>> {
  template <typename NtObjTy>
  static void write(boost::property_tree::ptree &pt, const NtObjTy &data) {
    boost::property_tree::ptree ptree;
    const std::vector<T> &vec = data.template get<CType>();
    for (auto item : vec) {
      boost::property_tree::ptree child = to_ptree<T>::f(item);
      ptree.push_back(std::make_pair("", child));
    }
    pt.add_child(name_traits<CType>::name, std::move(ptree));
  }
};

template <typename CType, typename... ARGS>
struct ptree_put_helper<CType, ff::util::ntobject<ARGS...>> {
  template <typename NtObjTy>
  static void write(boost::property_tree::ptree &pt, const NtObjTy &data) {
    boost::property_tree::ptree ptree =
        to_ptree<ff::util::ntobject<ARGS...>>::f(data.template get<CType>());
    pt.add_child(name_traits<CType>::name, std::move(ptree));
  }
};

template <typename CType, typename T>
struct ptree_get_helper<CType, std::vector<T>> {
  template <typename NtObjTy>
  static void read(const boost::property_tree::ptree &pt, NtObjTy &data) {
    if (pt.find(name_traits<CType>::name) == pt.not_found()) {
      LOG(WARNING) << "cannot find json item with name: "
                   << name_traits<CType>::name << ", use default empty vector.";
      return;
    }
    std::vector<T> d;
    auto ptree = pt.get_child(name_traits<CType>::name);
    for (auto it = ptree.begin(); it != ptree.end(); ++it) {
      d.push_back(from_ptree<T>::f(it->second));
    }

    data.template set<CType>(std::move(d));
  }
};
template <typename CType, typename... ARGS>
struct ptree_get_helper<CType, ff::util::ntobject<ARGS...>> {
  template <typename NtObjTy>
  static void read(const boost::property_tree::ptree &pt, NtObjTy &data) {
    if (pt.find(name_traits<CType>::name) == pt.not_found()) {
      LOG(WARNING) << "cannot find json item with name: "
                   << name_traits<CType>::name << ", use default empty vector.";
      return;
    }
    auto ptree = pt.get_child(name_traits<CType>::name);
    ff::util::ntobject<ARGS...> d =
        from_ptree<ff::util::ntobject<ARGS...>>::f(ptree);
    data.template set<CType>(std::move(d));
  };
};

template <int Index> struct to_json_helper {
  template <typename NtObjTy>
  static auto write(boost::property_tree::ptree &pt, const NtObjTy &data) ->
      typename std::enable_if<(NtObjTy::type_list::len > Index), void>::type {
    typedef typename ff::util::get_type_at_index_in_typelist<
        typename NtObjTy::type_list, Index>::type ctype;
    ptree_put_helper<ctype>::write(pt, data);
    to_json_helper<Index + 1>::write(pt, data);
  }
  template <typename NtObjTy>
  static auto write(boost::property_tree::ptree &pt, const NtObjTy &data) ->
      typename std::enable_if<(NtObjTy::type_list::len <= Index), void>::type {}
};

template <typename... ARGS> struct to_ptree<ff::util::ntobject<ARGS...>> {
  static boost::property_tree::ptree
  f(const ff::util::ntobject<ARGS...> &value) {
    boost::property_tree::ptree child;
    to_json_helper<0>::write(child, value);
    return child;
  }
};

template <int Index> struct from_json_helper {
  template <typename NtObjTy>
  static auto read(const boost::property_tree::ptree &pt, NtObjTy &data) ->
      typename std::enable_if<(NtObjTy::type_list::len > Index), void>::type {
    typedef typename ff::util::get_type_at_index_in_typelist<
        typename NtObjTy::type_list, Index>::type ctype;
    typedef typename ff::util::internal::nt_traits<ctype>::type vtype;
    ptree_get_helper<ctype>::read(pt, data);
    from_json_helper<Index + 1>::read(pt, data);
  }
  template <typename NtObjTy>
  static auto read(const boost::property_tree::ptree &pt, NtObjTy &data) ->
      typename std::enable_if<(NtObjTy::type_list::len <= Index), void>::type {}
};

template <typename... ARGS> struct from_ptree<ff::util::ntobject<ARGS...>> {
  static ff::util::ntobject<ARGS...>
  f(const boost::property_tree::ptree &ptree) {
    ff::util::ntobject<ARGS...> ret;
    from_json_helper<0>::read(ptree, ret);
    return ret;
  }
};

} // namespace internal

class ntjson {
public:
  template <typename NtObjTy> static std::string to_json(const NtObjTy &data) {
    boost::property_tree::ptree pt;
    internal::to_json_helper<0>::write(pt, data);
    std::stringstream ss;
    boost::property_tree::write_json(ss, pt);
    return ss.str();
  }
  template <typename NtObjTy>
  static void to_json_file(const NtObjTy &data, const std::string &path) {
    boost::property_tree::ptree pt;
    internal::to_json_helper<0>::write(pt, data);
    boost::property_tree::json_parser::write_json(path, pt);
  }

  template <typename NtObjTy>
  static NtObjTy from_json(const std::string &json_string) {
    std::stringstream ss;
    ss << json_string;
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);
    NtObjTy data;
    internal::from_json_helper<0>::read(pt, data);
    return data;
  }

  template <typename NtObjTy>
  static NtObjTy from_json_file(const std::string &path) {
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(path, pt);
    NtObjTy data;
    internal::from_json_helper<0>::read(pt, data);
    return data;
  }
};
} // namespace ypc

