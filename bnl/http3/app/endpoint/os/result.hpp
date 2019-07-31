#pragma once

#include <bnl/base/log.hpp>
#include <bnl/base/result.hpp>

#include <system_error>

enum class error {
  timeout,
  resolve,
  invalid_argument,
  idle,
  finished
};

const std::error_category &error_category() noexcept;

std::error_code make_error_code(error error) noexcept;

namespace std {
  template <> struct is_error_code_enum<error> : true_type {};
}

template<typename T>
using result = bnl::base::result<T, std::error_code>;

#define THROW_SYSTEM(function, errno)                                          \
  {                                                                            \
    std::error_code ec = { errno, std::system_category() };                    \
    BNL_LOG_E("{}: {}", #function, ec.message());                              \
    return ec;                                                                 \
  }                                                                            \
  (void) 0
