#pragma once

#include <bnl/base/export.hpp>
#include <bnl/base/result.hpp>

#include <cstdint>
#include <mutex>

namespace bnl {
namespace quic {

enum class error : uint64_t {
  no_error = 0x0,
  internal = 0x1,

  incomplete = 0x40ULL << 56U,
  idle,
  delegate,
  finished,
  not_implemented,
  handshake,
  crypto,
  ngtcp2,
  path_validation,
  stream_not_found,
  stream_id_blocked,
  stream_data_blocked
};

template <typename T>
using result = base::result<T, error>;

namespace application {

struct error {
  enum type { rst_stream, stop_sending, connection_close };

  const type type;
  const uint64_t id;
  const uint64_t value;
};

}

}
}
