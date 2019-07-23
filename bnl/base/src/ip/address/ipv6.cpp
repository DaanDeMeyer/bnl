#include <bnl/ip/address/ipv6.hpp>

#include <algorithm>
#include <cassert>
#include <ostream>

namespace bnl {
namespace ipv6 {

address::address(base::buffer_view bytes) noexcept
  : bytes_()
{
  assert(bytes.size() == size);
  std::copy_n(bytes.data(), bytes.size(), bytes_.data());
}

address::address(const uint8_t *bytes) noexcept
  : address({ bytes, size })
{}

base::buffer_view
address::bytes() const noexcept
{
  return { bytes_.data(), bytes_.size() };
}

std::ostream &
operator<<(std::ostream &os, const address &address)
{
  (void) address;
  return os << "Printing ipv6 is hard";
}

}
}
