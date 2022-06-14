#pragma once
namespace ypc {
namespace internal {

template <typename DataSession> struct is_multi_datasource {
  static constexpr bool value = false;
};
} // namespace internal
} // namespace ypc
