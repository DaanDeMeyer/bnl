#pragma once

#include <h3c/buffer.hpp>
#include <h3c/buffers.hpp>
#include <h3c/export.hpp>
#include <h3c/util/class.hpp>

#include <cstdint>
#include <system_error>

namespace h3c {

class logger;

namespace prefix_int {

class H3C_EXPORT encoder {
public:
  explicit encoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(encoder);

  ~encoder() = default;

  size_t encoded_size(uint64_t value, uint8_t prefix) const noexcept;

  size_t encode(uint8_t *dest, uint64_t value, uint8_t prefix) const noexcept;

  buffer encode(uint64_t value, uint8_t prefix) const;

private:
  logger *logger_;
};

class H3C_EXPORT decoder {
public:
  explicit decoder(logger *logger);

  H3C_MOVE_ONLY(decoder);

  ~decoder() = default;

  uint64_t
  decode(buffer &encoded, uint8_t prefix, std::error_code &ec) const noexcept;

  uint64_t
  decode(buffers &encoded, uint8_t prefix, std::error_code &ec) const noexcept;

private:
  logger *logger_;

  template <typename Sequence>
  uint64_t
  decode(Sequence &encoded, uint8_t prefix, std::error_code &ec) const noexcept;

  template <typename Sequence>
  uint8_t uint8_decode(Sequence &encoded, std::error_code &ec) const noexcept;
};

} // namespace prefix_int
} // namespace h3c