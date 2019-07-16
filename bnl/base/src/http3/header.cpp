#include <bnl/http3/header.hpp>

#include <ostream>

namespace bnl {
namespace http3 {

header_view::header_view(base::string_view name,
                         base::string_view value) noexcept
  : name_(name)
  , value_(value)
{}

base::string_view
header_view::name() const noexcept
{
  return name_;
}

base::string_view
header_view::value() const noexcept
{
  return value_;
}

bool
operator==(header_view first, header_view second) noexcept
{
  return first.name() == second.name() && first.value() == second.value();
}

bool
operator!=(header_view first, header_view second) noexcept
{
  return !(first == second);
}

std::ostream &
operator<<(std::ostream &os, header_view header)
{
  return os << header.name() << ": " << header.value();
}

header::header(base::string name, base::string value) noexcept
  : name_(std::move(name))
  , value_(std::move(value))
{}

base::string_view
header::name() const noexcept
{
  return name_;
}

base::string_view
header::value() const noexcept
{
  return value_;
}

header::operator header_view() const noexcept
{
  return { name(), value() };
}

} // namespace http3
} // namespace bnl
