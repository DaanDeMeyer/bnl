#pragma once

#include <bnl/core/export.hpp>

#include <bnl/string.hpp>
#include <bnl/string_view.hpp>

namespace bnl {
namespace http3 {

class BNL_CORE_EXPORT header_view {
public:
  header_view() = default;
  header_view(string_view name, string_view value) noexcept;

  string_view name() const noexcept;
  string_view value() const noexcept;

private:
  string_view name_;
  string_view value_;
};

BNL_CORE_EXPORT bool operator==(header_view first, header_view second) noexcept;
BNL_CORE_EXPORT bool operator!=(header_view first, header_view second) noexcept;

class BNL_CORE_EXPORT header {
public:
  header() = default;
  header(string name, string value) noexcept;

  string_view name() const noexcept;
  string_view value() const noexcept;

  operator header_view() const noexcept; // NOLINT

private:
  string name_;
  string value_;
};

} // namespace http3
} // namespace bnl
