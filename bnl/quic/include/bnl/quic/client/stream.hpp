#pragma once

#include <bnl/base/buffers.hpp>
#include <bnl/base/result.hpp>
#include <bnl/quic/error.hpp>
#include <bnl/quic/export.hpp>

#include <system_error>

namespace bnl {

namespace log {
class api;
}

namespace quic {
namespace client {

namespace ngtcp2 {
class connection;
}

class BNL_QUIC_EXPORT stream {
public:
  stream(uint64_t id, ngtcp2::connection *connection, const log::api *logger);

  base::result<base::buffer> send();

  std::error_code add(base::buffer buffer);
  std::error_code fin();

  std::error_code ack(size_t size);

  bool finished() const noexcept;

private:
  base::buffers buffers_;
  base::buffers keepalive_;
  bool fin_ = false;

  uint64_t id_;
  ngtcp2::connection *connection_;
  const log::api *logger_;
};

}
}
}
