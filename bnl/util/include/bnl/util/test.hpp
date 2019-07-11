#pragma once

template <typename E>
auto extract(E expression) -> typename decltype(expression())::value_type
{
  auto result = expression();
  REQUIRE(result);
  return result.value();
}

#define EXTRACT(...) extract([&]() { return __VA_ARGS__; });
