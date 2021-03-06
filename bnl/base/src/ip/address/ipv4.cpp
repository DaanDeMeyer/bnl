#include <bnl/ip/address/ipv4.hpp>

#include <algorithm>
#include <cassert>
#include <ostream>

namespace bnl {
namespace ipv4 {

address::address(base::buffer_view bytes) noexcept
  : bytes_()
{
  assert(bytes.size() == size);
  std::copy_n(bytes.data(), bytes.size(), bytes_.data());
}

address::address(const uint8_t *bytes) noexcept
  : address({ bytes, size })
{}

address::address(uint32_t bytes) noexcept
  : bytes_()
{
  bytes_[0] = static_cast<uint8_t>(bytes >> 0U);
  bytes_[1] = static_cast<uint8_t>(bytes >> 8U);
  bytes_[2] = static_cast<uint8_t>(bytes >> 16U);
  bytes_[3] = static_cast<uint8_t>(bytes >> 24U);
}

base::buffer_view
address::bytes() const noexcept
{
  return { bytes_.data(), bytes_.size() };
}

std::ostream &
operator<<(std::ostream &os, const address &address)
{
  const uint8_t *bytes = address.bytes().data();
  return os << +bytes[0] << "." << +bytes[1] << "." << +bytes[2] << "."
            << +bytes[3];
}

}
}
