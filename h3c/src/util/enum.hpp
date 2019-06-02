#pragma once

#include <type_traits>

namespace h3c {
namespace util {

template <typename E>
constexpr auto to_underlying(E e) -> typename std::underlying_type<E>::type
{
  return static_cast<typename std::underlying_type<E>::type>(e);
}

} // namespace util
} // namespace h3c
