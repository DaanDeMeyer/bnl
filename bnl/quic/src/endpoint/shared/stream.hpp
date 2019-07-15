#pragma once

#include <bnl/base/buffers.hpp>
#include <bnl/quic/error.hpp>

#include <system_error>

typedef struct ngtcp2_conn ngtcp2_conn;

namespace bnl {

namespace log {
class api;
}

namespace quic {

class stream
{
public:
  stream(uint64_t id, ngtcp2_conn* connection, const log::api* logger);

  std::error_code add(base::buffer buffer);
  std::error_code fin();

  std::error_code close(quic::error error);

private:
  base::buffers buffers_;
  bool fin_;

  const uint64_t id_;
  ngtcp2_conn* connection_;
  const log::api* logger_;
};

} // namespace quic
} // namespace bnl
