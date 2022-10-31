#pragma once
#include <type_traits>

#define decl_feature_helper(feature, funcname, ret, ...)                       \
  template <typename, typename T> struct has_##feature {                       \
    static constexpr bool value = false;                                       \
  };                                                                           \
  template <typename C, typename Ret, typename... Args>                        \
  struct has_##feature<C, Ret(Args...)> {                                      \
  private:                                                                     \
    template <typename T>                                                      \
    static constexpr auto check(T *) -> typename std::is_same<                 \
        decltype(std::declval<T>().funcname(std::declval<Args>()...)),         \
        Ret>::type;                                                            \
                                                                               \
    template <typename> static constexpr std::false_type check(...);           \
                                                                               \
    typedef decltype(check<C>(0)) type;                                        \
                                                                               \
  public:                                                                      \
    static constexpr bool value = type::value;                                 \
  };                                                                           \
  template <typename C, bool contains_##feature =                              \
                            has_##feature<C, ret(__VA_ARGS__)>::value>         \
  struct call_##feature##_helper {                                             \
    template <typename... ARGS> static ret call(C &obj, ARGS &&... args) {     \
      return obj.funcname(args...);                                            \
    }                                                                          \
  };                                                                           \
  template <typename C> struct call_##feature##_helper<C, false> {             \
    template <typename... ARGS> static ret call(C &obj, ARGS &&... args) {     \
      return stbox::stx_status::no_such_feature;                               \
    }                                                                          \
  };

decl_feature_helper(init_model, init_model, uint32_t, const uint8_t *,
                    uint32_t);
decl_feature_helper(init_data_source, init_data_source, uint32_t,
                    const uint8_t *, uint32_t);

