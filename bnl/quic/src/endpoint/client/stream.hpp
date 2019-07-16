#pragma once

#include <bnl/base/buffers.hpp>
#include <bnl/base/result.hpp>
#include <bnl/quic/error.hpp>

#include <system_error>

namespace bnl {

namespace log {
class api;
}

namespace quic {
namespace endpoint {
namespace client {

namespace ngtcp2 {
class connection;
}

class stream {
public:
  stream(uint64_t id, ngtcp2::connection *connection, const log::api *logger);

  base::result<base::buffer> send();

  std::error_code add(base::buffer buffer);
  std::error_code fin();

private:
  base::buffers buffers_;
  base::buffers keepalive_;
  bool fin_;

  uint64_t id_;
  ngtcp2::connection *connection_;
  const log::api *logger_;
};

} // namespace quic
} // namespace bnl
}
}
