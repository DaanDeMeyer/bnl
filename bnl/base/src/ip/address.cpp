#include <bnl/ip/address.hpp>

#include <cassert>

namespace bnl {
namespace ip {

address::address()
  : type_(type::ipv4)
  , ipv4()
{
}

address::address(ipv4::address address) noexcept
  : type_(type::ipv4)
  , ipv4(address)
{
}

address::address(ipv6::address address) noexcept
  : type_(type::ipv6)
  , ipv6(address)
{
}

address::operator type() const noexcept
{
  return type_;
}

base::buffer_view
address::bytes() const noexcept
{
  switch (type_) {
    case type::ipv4:
      return ipv4.bytes();
    case type::ipv6:
      return ipv6.bytes();
  }

  assert(false);
  return {};
}

} // namespace ip
} // namespace bnl
