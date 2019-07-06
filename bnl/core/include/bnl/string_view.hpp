#pragma once

#include <bnl/core/export.hpp>

#include <bnl/string.hpp>

#include <cstddef>
#include <iosfwd>

namespace bnl {

class BNL_CORE_EXPORT string_view {
public:
  string_view() = default;
  string_view(const char *data, size_t size) noexcept;

  string_view(const string &string) noexcept; // NOLINT

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

BNL_CORE_EXPORT bool operator==(string_view lhs, string_view rhs) noexcept;
BNL_CORE_EXPORT bool operator!=(string_view lhs, string_view rhs) noexcept;

BNL_CORE_EXPORT std::ostream &operator<<(std::ostream &out, string_view string);

} // namespace bnl
