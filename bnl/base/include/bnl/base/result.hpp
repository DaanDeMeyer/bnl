#pragma once

#include <bnl/base/macro.hpp>

#include <system_error>
#include <utility>

namespace bnl {
namespace base {

template <typename T>
class result {
public:
  using value_type = T;

  result(T &&value) : value_(std::forward<T>(value)) {} // NOLINT
  result(std::error_code ec) : ec_(ec) {}               // NOLINT

  value_type &value() &
  {
    if (ec_) {
      throw std::system_error(ec_);
    }

    return value_;
  }

  value_type &&value() &&
  {
    if (ec_) {
      throw std::system_error(ec_);
    }

    return std::move(value_);
  }

  std::error_code error() const noexcept
  {
    return ec_;
  }

  explicit operator bool() const noexcept
  {
    return !ec_;
  }

private:
  T value_;
  std::error_code ec_;
};

template <typename T>
bool operator==(const result<T> &lhs, std::error_code rhs)
{
  return lhs.error() == rhs;
}

template <typename T>
bool operator!=(const result<T> &lhs, std::error_code rhs)
{
  return !(lhs == rhs);
}

template <typename T>
bool operator==(const result<T> &lhs, const std::error_condition &rhs)
{
  return lhs.error() == rhs;
}

template <typename T>
bool operator!=(const result<T> &lhs, const std::error_condition &rhs)
{
  return !(lhs == rhs);
}

template <typename T>
bool BNL_TRY_IS_ERROR(const T &result)
{
  return !result;
}

template <>
inline bool BNL_TRY_IS_ERROR(const std::error_code &result)
{
  return result.value() > 0;
}

template <typename T>
std::error_code BNL_TRY_GET_ERROR(T &&result)
{
  return result.error();
}

template <>
inline std::error_code BNL_TRY_GET_ERROR(std::error_code &&result)
{
  return result;
}

} // namespace base
} // namespace bnl

#define BNL_TRY(...)                                                           \
  ({                                                                           \
    auto &&res = (__VA_ARGS__);                                                \
    if (bnl::base::BNL_TRY_IS_ERROR(res)) {                                    \
      return bnl::base::BNL_TRY_GET_ERROR(std::move(res));                     \
    }                                                                          \
                                                                               \
    std::move(res).value();                                                    \
  })
