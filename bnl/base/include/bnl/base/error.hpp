#pragma once

#include <bnl/base/export.hpp>

#include <system_error>

namespace bnl {
namespace base {

enum class error {
  internal,
  not_implemented,
  invalid_argument,
  incomplete,
  idle,
  unknown
};

BNL_BASE_EXPORT const std::error_category &error_category() noexcept;

BNL_BASE_EXPORT std::error_code make_error_code(error error) noexcept;

} // namespace base
} // namespace bnl

namespace std {

template <>
struct is_error_code_enum<bnl::base::error> : true_type {};

} // namespace std
