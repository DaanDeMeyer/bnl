#include <bnl/ip/address.hpp>

#include <cassert>

namespace bnl {
namespace ip {

static address::type
size_to_type(size_t size)
{
  assert(size == ipv4::address::size || size == ipv6::address::size);
  return size == ipv4::address::size ? address::type::ipv4
                                     : address::type::ipv6;
}

address::address()
  : type_(type::ipv4)
  , ipv4_()
{}

address::address(ipv4::address address) noexcept
  : type_(type::ipv4)
  , ipv4_(address)
{}

address::address(ipv6::address address) noexcept
  : type_(type::ipv6)
  , ipv6_(address)
{}

address::address(base::buffer_view bytes) noexcept
  : type_(size_to_type(bytes.size()))
{
  switch (type_) {
    case type::ipv4:
      new (&ipv4_) decltype(ipv4_)(bytes);
      break;
    case type::ipv6:
      new (&ipv6_) decltype(ipv6_)(bytes);
      break;
    default:
      assert(false);
  }
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
      return ipv4_.bytes();
    case type::ipv6:
      return ipv6_.bytes();
  }

  assert(false);
  return {};
}

std::ostream &
operator<<(std::ostream &os, const address &address)
{
  switch (address) {
    case address::type::ipv4:
      return os << address.ipv4_;
    case address::type::ipv6:
      return os << address.ipv6_;
  }

  assert(false);
  return os;
}

bool
operator==(const address &lhs, const address &rhs)
{
  return lhs.bytes() == rhs.bytes();
}

}
}
