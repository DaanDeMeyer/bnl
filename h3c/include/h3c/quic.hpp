#pragma once

#include <h3c/buffer.hpp>

#include <cstdint>

namespace h3c {

namespace quic {

struct data {
  uint64_t id;
  h3c::buffer buffer;
  bool fin;
};

struct error {
  enum class type { connection, stream };

  uint64_t id;
  type type;
  uint64_t code;
};

} // namespace quic
} // namespace h3c