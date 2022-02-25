#pragma once
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <ff/net/middleware/ntpackage.h>
#include <ff/util/ntobject.h>

#define DECL_TYPENAME(z, n, text) typename text##n
#define DECL_TYPE(z, n, text) text##n
#define DECL_VAR_DECL(z, n, text)                                              \
  typename ::ff::util::internal::nt_traits<text##n>::type t##n;
#define DECL_VAR(z, n, text) text##n

#define AH(z, num, _)                                                          \
  template <BOOST_PP_ENUM(BOOST_PP_INC(num), DECL_TYPENAME, T)>                \
  struct assign_helper<                                                        \
      ::ff::util::ntobject<BOOST_PP_ENUM(BOOST_PP_INC(num), DECL_TYPE, T)>> {  \
    template <typename RT>                                                     \
    static bool read_row(                                                      \
        RT *reader,                                                            \
        ::ff::util::ntobject<BOOST_PP_ENUM(BOOST_PP_INC(num), DECL_TYPE, T)>   \
            &v) {                                                              \
      BOOST_PP_REPEAT(BOOST_PP_INC(num), DECL_VAR_DECL, T);                    \
      bool rv =                                                                \
          reader->read_row(BOOST_PP_ENUM(BOOST_PP_INC(num), DECL_VAR, t));     \
      if (!rv) {                                                               \
        return false;                                                          \
      }                                                                        \
      v.template set<BOOST_PP_ENUM(BOOST_PP_INC(num), DECL_TYPE, T)>(          \
          BOOST_PP_ENUM(BOOST_PP_INC(num), DECL_VAR, t));                      \
      return rv;                                                               \
    }                                                                          \
  };                                                                           \
                                                                               \
  template <uint32_t PackageID,                                                \
            BOOST_PP_ENUM(BOOST_PP_INC(num), DECL_TYPENAME, T)>                \
  struct assign_helper<::ff::net::ntpackage<                                   \
      PackageID, BOOST_PP_ENUM(BOOST_PP_INC(num), DECL_TYPE, T)>> {            \
    template <typename RT>                                                     \
    static bool read_row(                                                      \
        RT *reader,                                                            \
        ::ff::net::ntpackage<PackageID, BOOST_PP_ENUM(BOOST_PP_INC(num),       \
                                                      DECL_TYPE, T)> &v) {     \
      BOOST_PP_REPEAT(BOOST_PP_INC(num), DECL_VAR_DECL, T);                    \
      bool rv =                                                                \
          reader->read_row(BOOST_PP_ENUM(BOOST_PP_INC(num), DECL_VAR, t));     \
      if (!rv) {                                                               \
        return false;                                                          \
      }                                                                        \
      v.template set<BOOST_PP_ENUM(BOOST_PP_INC(num), DECL_TYPE, T)>(          \
          BOOST_PP_ENUM(BOOST_PP_INC(num), DECL_VAR, t));                      \
      return true;                                                             \
    }                                                                          \
  };

namespace ypc {
namespace plugins {
namespace internal {
template <typename NT> struct assign_helper {};
// AH(1, 1, nil)
BOOST_PP_REPEAT(64, AH, nil)
} // namespace internal
} // namespace plugins
} // namespace ypc
