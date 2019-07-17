#include <bnl/ip/endpoint.hpp>

namespace bnl {
namespace ip {

endpoint::endpoint(ip::address address, uint16_t port) noexcept
  : address_(address)
  , port_(port)
{}

const ip::address &
endpoint::address() const noexcept
{
  return address_;
}

uint16_t
endpoint::port() const noexcept
{
  return port_;
}

}
}
