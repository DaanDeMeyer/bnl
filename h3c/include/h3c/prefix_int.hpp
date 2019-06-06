#pragma once

#include <h3c/buffer.hpp>
#include <h3c/export.hpp>
#include <h3c/util/class.hpp>

#include <cstdint>
#include <system_error>

namespace h3c {

class logger;

namespace prefix_int {

class encoder {
public:
  explicit encoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(encoder)

  H3C_EXPORT size_t encoded_size(uint64_t value, uint8_t prefix) const noexcept;

  H3C_EXPORT size_t encode(uint8_t *dest, uint64_t value, uint8_t prefix) const
      noexcept;

  H3C_EXPORT buffer encode(uint64_t value, uint8_t prefix) const;

private:
  logger *logger_;
};

class decoder {
public:
  explicit decoder(logger *logger);

  H3C_MOVE_ONLY(decoder)

  H3C_EXPORT uint64_t decode(buffer &encoded,
                             uint8_t prefix,
                             std::error_code &ec) const noexcept;

private:
  logger *logger_;

  uint8_t uint8_decode(buffer &encoded, std::error_code &ec) const noexcept;
};

} // namespace prefix_int
} // namespace h3c