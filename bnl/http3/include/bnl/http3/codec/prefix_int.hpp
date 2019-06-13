#pragma once

#include <bnl/http3/export.hpp>

#include <bnl/buffer.hpp>
#include <bnl/buffers.hpp>

#include <cstdint>
#include <system_error>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace prefix_int {

class BNL_HTTP3_EXPORT encoder {
public:
  explicit encoder(const log::api *logger) noexcept;

  size_t encoded_size(uint64_t value, uint8_t prefix) const noexcept;

  size_t encode(uint8_t *dest, uint64_t value, uint8_t prefix) const noexcept;

  buffer encode(uint64_t value, uint8_t prefix) const;

private:
  const log::api *logger_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger);

  uint64_t
  decode(buffer &encoded, uint8_t prefix, std::error_code &ec) const noexcept;

  uint64_t
  decode(buffers &encoded, uint8_t prefix, std::error_code &ec) const noexcept;

private:
  const log::api *logger_;

  template <typename Sequence>
  uint64_t
  decode(Sequence &encoded, uint8_t prefix, std::error_code &ec) const noexcept;

  template <typename Sequence>
  uint8_t uint8_decode(Sequence &encoded, std::error_code &ec) const noexcept;
};

} // namespace prefix_int
} // namespace http3
} // namespace bnl
