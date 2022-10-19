#pragma once
namespace ypc {
namespace internal {

template <typename Result> struct is_param_encrypted {
  static constexpr bool value = true;
};
} // namespace internal
} // namespace ypc
