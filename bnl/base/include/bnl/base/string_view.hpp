#pragma once

#include <bnl/base/export.hpp>
#include <bnl/base/string.hpp>

#include <cstddef>
#include <iosfwd>

namespace bnl {
namespace base {

class BNL_BASE_EXPORT string_view {
public:
  string_view() = default;

  string_view(const char *data) noexcept; // NOLINT
  string_view(const char *data, size_t size) noexcept;
  string_view(const string &string) noexcept; // NOLINT

  string_view(const string_view &other) = default;
  string_view &operator=(const string_view &other) = default;

  const char *data() const noexcept;

  size_t size() const noexcept;
  bool empty() const noexcept;

  const char *begin() const noexcept;
  const char *end() const noexcept;

  char operator[](size_t index) const noexcept;
  char operator*() const noexcept;

private:
  const char *data_ = nullptr;
  size_t size_ = 0;
};

BNL_BASE_EXPORT bool
operator==(string_view lhs, string_view rhs) noexcept;
BNL_BASE_EXPORT bool
operator!=(string_view lhs, string_view rhs) noexcept;

BNL_BASE_EXPORT std::ostream &
operator<<(std::ostream &out, string_view string);

}
}
