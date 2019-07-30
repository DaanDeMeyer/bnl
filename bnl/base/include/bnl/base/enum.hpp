#pragma once

#include <type_traits>

namespace bnl {
namespace enumeration {

template<typename Enumeration>
typename std::underlying_type<Enumeration>::type
value(Enumeration value)
{
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

}
}
