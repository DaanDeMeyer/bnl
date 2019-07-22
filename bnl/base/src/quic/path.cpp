#include <bnl/quic/path.hpp>

namespace bnl {
namespace quic {

path::path(ip::endpoint local, ip::endpoint peer) noexcept
  : local_(local)
  , peer_(peer)
{}

ip::endpoint
path::local() const noexcept
{
  return local_;
}

ip::endpoint
path::peer() const noexcept
{
  return peer_;
}

}
}
