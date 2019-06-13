#pragma once

#include <type_traits>

namespace bnl {
namespace util {

template <typename E>
constexpr auto // NOLINT
to_underlying(E e) -> typename std::underlying_type<E>::type
{
  return static_cast<typename std::underlying_type<E>::type>(e);
}

} // namespace util
} // namespace bnl
