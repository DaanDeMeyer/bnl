#include <bnl/quic/path.hpp>

namespace bnl {
namespace quic {

path::path(ip::endpoint local, ip::endpoint peer) noexcept
  : local_(local)
  , peer_(peer)
{}

const ip::endpoint &
path::local() const noexcept
{
  return local_;
}

const ip::endpoint &
path::peer() const noexcept
{
  return peer_;
}

}
}
