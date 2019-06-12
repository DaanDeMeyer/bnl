#pragma once

#include <h3c/buffer.hpp>
#include <h3c/buffers.hpp>
#include <h3c/export.hpp>
#include <h3c/util/class.hpp>

#include <cstddef>
#include <cstdint>
#include <system_error>

namespace h3c {

class logger;

namespace huffman {

class H3C_EXPORT encoder {
public:
  explicit encoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(encoder);

  ~encoder() = default;

  size_t encoded_size(buffer_view string) const noexcept;

  size_t encode(uint8_t *dest, buffer_view string) const noexcept;

  buffer encode(buffer_view string) const;

private:
  logger *logger_;
};

class H3C_EXPORT decoder {
public:
  explicit decoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(decoder);

  ~decoder() = default;

  buffer
  decode(buffer &encoded, size_t encoded_size, std::error_code &ec) const;

  buffer
  decode(buffers &encoded, size_t encoded_size, std::error_code &ec) const;

private:
  logger *logger_;

  template <typename Sequence>
  buffer
  decode(Sequence &encoded, size_t encoded_size, std::error_code &ec) const;

  template <typename Sequence>
  size_t decoded_size(Sequence &encoded,
                      size_t encoded_size,
                      std::error_code &ec) const noexcept;
};

} // namespace huffman

} // namespace h3c
