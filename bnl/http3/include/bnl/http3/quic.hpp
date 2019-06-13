#pragma once

#include <bnl/http3/export.hpp>

#include <bnl/buffer.hpp>

#include <cstdint>

namespace bnl {
namespace http3 {
namespace quic {

struct BNL_HTTP3_EXPORT data {
  data() = default;
  data(uint64_t id, bnl::buffer buffer, bool fin)
      : id(id), buffer(std::move(buffer)), fin(fin)
  {}

  uint64_t id = 0;
  bnl::buffer buffer;
  bool fin = false;
};

struct BNL_HTTP3_EXPORT error {
  enum class type { connection, stream };

  error() = default;
  error(uint64_t id, type type, uint64_t code) : id(id), type(type), code(code)
  {}

  uint64_t id = 0;
  type type = type::connection;
  uint64_t code = 0;
};

} // namespace quic
} // namespace http3
} // namespace bnl
