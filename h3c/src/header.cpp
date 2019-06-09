#include <h3c/header.hpp>

namespace h3c {

header_view::header_view(buffer_view name, buffer_view value)
    : name_(name), value_(value)
{}

buffer_view header_view::name() const noexcept
{
  return name_;
}

buffer_view header_view::value() const noexcept
{
  return value_;
}

header::operator header_view() const noexcept
{
  return { { name.data(), name.size() }, { value.data(), value.size() } };
}

bool operator==(header_view first, header_view second) noexcept
{
  return first.name() == second.name() && first.value() == second.value();
}

bool operator!=(header_view first, header_view second) noexcept
{
  return !(first == second);
}

} // namespace h3c
