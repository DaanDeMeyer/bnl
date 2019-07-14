#pragma once

#include <bnl/base/error.hpp>
#include <bnl/base/macro.hpp>

#include <system_error>
#include <utility>

namespace bnl {
namespace base {

template <typename T>
class result {
public:
  using value_type = T;

  result(T &&value) noexcept // NOLINT
      : type_(type::value), value_(std::forward<T>(value))
  {}

  result(std::error_code ec) noexcept : type_(type::error), ec_(ec) {} // NOLINT

  BNL_BASE_NO_COPY(result);

  result(result<T> &&other) noexcept : type_(type::error), ec_()
  {
    operator=(std::move(other));
  }

  result<T> &operator=(result<T> &&other) noexcept
  {
    if (&other != this) {
      destroy();

      type_ = other.type_;

      switch (type_) {
        case type::value:
          new (&value_) T(std::move(other.value()));
          break;
        case type::error:
          new (&ec_) std::error_code(other.ec_);
          break;
      }
    }

    return *this;
  };

  ~result()
  {
    destroy();
  }

  value_type &value() &
  {
    if (type_ == type::error) {
      throw std::system_error(ec_);
    }

    return value_;
  }

  value_type &&value() &&
  {
    if (type_ == type::error) {
      throw std::system_error(ec_);
    }

    return std::move(value_);
  }

  std::error_code error() const noexcept
  {
    return type_ == type::value ? base::error::success : ec_;
  }

  explicit operator bool() const noexcept
  {
    return type_ == type::value;
  }

private:
  void destroy()
  {
    switch (type_) {
      case type::value:
        value_.~T();
        break;
      case type::error:
        ec_.~error_code();
        break;
    }
  }

private:
  enum class type : uint8_t { value, error };

  type type_;

  union {
    T value_;
    std::error_code ec_;
  };
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
std::error_code BNL_TRY_GET_ERROR(const T &result)
{
  return result.error();
}

template <>
inline std::error_code BNL_TRY_GET_ERROR(const std::error_code &result)
{
  return result;
}

} // namespace base
} // namespace bnl

#define BNL_TRY(...)                                                           \
  ({                                                                           \
    auto &&res = (__VA_ARGS__);                                                \
    if (bnl::base::BNL_TRY_IS_ERROR(res)) {                                    \
      return bnl::base::BNL_TRY_GET_ERROR(res);                                \
    }                                                                          \
                                                                               \
    std::move(res).value();                                                    \
  })
