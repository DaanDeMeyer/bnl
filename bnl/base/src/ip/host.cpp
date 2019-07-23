#include <bnl/ip/host.hpp>

#include <cstring>

namespace bnl {
namespace ip {

host::host(base::string name) noexcept
  : name_(std::move(name))
{}

host::host(const char *name) noexcept
  : name_(name, strlen(name))
{}

base::string_view
host::name() const noexcept
{
  return { name_.data(), name_.size() };
}

std::ostream &
operator<<(std::ostream &os, const host &host)
{
  return os << host.name();
}

}
}
