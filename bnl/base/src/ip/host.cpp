#include <bnl/ip/host.hpp>

namespace bnl {
namespace ip {

host::host(std::string name) noexcept
  : name_(std::move(name))
{
}

base::string_view
host::name() const noexcept
{
  return { name_.data(), name_.size() };
}

} // namespace ip
} // namespace bnl
