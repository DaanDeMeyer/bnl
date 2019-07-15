#include <bnl/quic/path.hpp>

namespace bnl {
namespace quic {

path::path(ip::endpoint local, ip::endpoint remote) noexcept
  : local_(local)
  , remote_(remote)
{
}

ip::endpoint
path::local() const noexcept
{
  return local_;
}

ip::endpoint
path::remote() const noexcept
{
  return remote_;
}

} // namespace quic
} // namespace bnl
