#pragma once

#include <h3c/buffer.hpp>
#include <h3c/export.hpp>
#include <h3c/util/class.hpp>

#include <cstddef>
#include <cstdint>
#include <system_error>

namespace h3c {

class logger;

namespace huffman {

class encoder {
public:
  H3C_EXPORT explicit encoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(encoder)

  H3C_EXPORT size_t encoded_size(buffer_view string) const noexcept;

  H3C_EXPORT size_t encode(uint8_t *dest, buffer_view string) const noexcept;

  H3C_EXPORT buffer encode(buffer_view string) const;

private:
  logger *logger_;
};

class decoder {
public:
  H3C_EXPORT explicit decoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(decoder)

  H3C_EXPORT buffer decode(buffer &encoded,
                           size_t encoded_size,
                           std::error_code &ec) const;

private:
  logger *logger_;

  size_t decoded_size(const buffer &encoded,
                      size_t encoded_size,
                      std::error_code &ec) const noexcept;
};

} // namespace huffman

} // namespace h3c
