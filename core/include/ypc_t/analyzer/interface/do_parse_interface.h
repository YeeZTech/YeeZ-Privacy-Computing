#pragma once
#include "stbox/ebyte.h"
#include "ypc_t/analyzer/var/model_var.h"
#include "ypc_t/analyzer/var/parser_var.h"
#include <type_traits>

namespace ypc {
namespace internal {

template <typename, typename T> struct has_set_context {
  static constexpr bool value = false;
};

template <typename C, typename Ret, typename... Args>
struct has_set_context<C, Ret(Args...)> {
private:
  template <typename T>
  static constexpr auto check(T *) -> typename std::is_same<
      decltype(std::declval<T>().set_context(std::declval<Args>()...)),
      Ret>::type;

  template <typename> static constexpr std::false_type check(...);

  typedef decltype(check<C>(0)) type;

public:
  static constexpr bool value = type::value;
};

template <typename, typename T> struct has_do_parse {
  static constexpr bool value = false;
  // static_assert(
  // std::integral_constant<T, false>::value,
  //"A parser must contains one of the following public function: "
  //"\n1. stbox::bytes do_parse(const stbox::bytes & param) "
  //"\n2. stbox::bytes do_parse(const uint8_t * param, uint32_t len) "
  //"\n3. template<typename Model> stbox::bytes do_parse(const Model & "
  //"\tmodel, const stbox::bytes & param)"
  //"\n4. template<typename Model> stbox::bytes do_parse(const Model & "
  //"model, const uint8_t * param, uint32_t len)");
};

template <typename C, typename Ret, typename... Args>
struct has_do_parse<C, Ret(Args...)> {
private:
  template <typename T>
  static constexpr auto check(T *) -> typename std::is_same<
      decltype(std::declval<T>().do_parse(std::declval<Args>()...)), Ret>::type;

  template <typename> static constexpr std::false_type check(...);

  typedef decltype(check<C>(0)) type;

public:
  static constexpr bool value = type::value;
};

template <typename ModelT> struct get_raw_param_func {
  typedef stbox::bytes type(const ModelT &, const uint8_t *, uint32_t);
};
template <> struct get_raw_param_func<void> {
  typedef stbox::bytes type(const uint8_t *, uint32_t);
};
template <typename ParserT,
          bool has_set_context_func =
              has_set_context<ParserT, void(analyzer_context *)>::value>
struct set_analyzer_context_helper {
  static void set(ParserT *parser, analyzer_context *context) {
    parser->set_context(context);
  }
};
template <typename ParserT> struct set_analyzer_context_helper<ParserT, false> {
  static void set(ParserT *parser, analyzer_context *context) {}
};

template <typename ParserT, typename ModelT,
          bool hash_model = !std::is_same<ModelT, void>::value,
          bool use_raw_param = has_do_parse<
              ParserT, typename get_raw_param_func<ModelT>::type>::value>
class do_parse_interface {
};

template <typename ParserT>
class do_parse_interface<ParserT, void, false, true>
    : virtual public parser_var<ParserT>, virtual public analyzer_context {
  typedef parser_var<ParserT> parser_var_t;
protected:
  stbox::bytes do_parse(const uint8_t *input_param, uint32_t len) {
    set_analyzer_context_helper<ParserT>::set(parser_var_t::m_parser.get(),
                                              this);
    return parser_var_t::m_parser->do_parse(input_param, len);
  }
};

template <typename ParserT>
class do_parse_interface<ParserT, void, false, false>
    : virtual public parser_var<ParserT>, virtual public analyzer_context {
  typedef parser_var<ParserT> parser_var_t;

protected:
  stbox::bytes do_parse(const uint8_t *input_param, uint32_t len) {
    set_analyzer_context_helper<ParserT>::set(parser_var_t::m_parser.get(),
                                              this);
    return parser_var_t::m_parser->do_parse(stbox::bytes(input_param, len));
  }
};

template <typename ParserT, typename ModelT>
class do_parse_interface<ParserT, ModelT, true, true>
    : virtual public model_var<ModelT>,
      virtual public parser_var<ParserT>,
      virtual public analyzer_context {
  typedef parser_var<ParserT> parser_var_t;
  typedef model_var<ModelT> model_var_t;

protected:
  stbox::bytes do_parse(const uint8_t *input_param, uint32_t len) {
    set_analyzer_context_helper<ParserT>::set(parser_var_t::m_parser.get(),
                                              this);
    return parser_var_t::m_parser->do_parse(model_var_t::m_model, input_param,
                                            len);
  }
};

template <typename ParserT, typename ModelT>
class do_parse_interface<ParserT, ModelT, true, false>
    : virtual public model_var<ModelT>,
      virtual public parser_var<ParserT>,
      virtual public analyzer_context {
  typedef parser_var<ParserT> parser_var_t;
  typedef model_var<ModelT> model_var_t;

protected:
  stbox::bytes do_parse(const uint8_t *input_param, uint32_t len) {
    set_analyzer_context_helper<ParserT>::set(parser_var_t::m_parser.get(),
                                              this);
    return parser_var_t::m_parser->do_parse(model_var_t::m_model,
                                            stbox::bytes(input_param, len));
  }
};
} // namespace internal
} // namespace ypc
