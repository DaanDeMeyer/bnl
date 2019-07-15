#pragma once

#include <bnl/base/export.hpp>
#include <bnl/base/string.hpp>
#include <bnl/base/string_view.hpp>

#include <iosfwd>

namespace bnl {
namespace http3 {

class BNL_BASE_EXPORT header_view
{
public:
  header_view() = default;
  header_view(base::string_view name, base::string_view value) noexcept;

  base::string_view name() const noexcept;
  base::string_view value() const noexcept;

private:
  base::string_view name_;
  base::string_view value_;
};

BNL_BASE_EXPORT bool
operator==(header_view first, header_view second) noexcept;
BNL_BASE_EXPORT bool
operator!=(header_view first, header_view second) noexcept;

BNL_BASE_EXPORT std::ostream&
operator<<(std::ostream& os, header_view header);

class BNL_BASE_EXPORT header
{
public:
  header() = default;
  header(base::string name, base::string value) noexcept;

  base::string_view name() const noexcept;
  base::string_view value() const noexcept;

  operator header_view() const noexcept; // NOLINT

private:
  base::string name_;
  base::string value_;
};

} // namespace http3
} // namespace bnl
