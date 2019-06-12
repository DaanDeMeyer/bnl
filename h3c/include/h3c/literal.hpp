#pragma once

#include <h3c/buffer.hpp>
#include <h3c/export.hpp>
#include <h3c/huffman.hpp>
#include <h3c/prefix_int.hpp>
#include <h3c/util/class.hpp>

#include <cstddef>
#include <cstdint>

namespace h3c {

class logger;

namespace literal {

class encoder {
public:
  explicit encoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(encoder);

  ~encoder() = default;

  H3C_EXPORT size_t encoded_size(buffer_view literal, uint8_t prefix) const
      noexcept;

  H3C_EXPORT size_t encode(uint8_t *dest,
                           buffer_view literal,
                           uint8_t prefix) const noexcept;

  H3C_EXPORT buffer encode(buffer_view literal, uint8_t prefix) const;

private:
  logger *logger_;

  prefix_int::encoder prefix_int_;
  huffman::encoder huffman_;
};

class decoder {
public:
  explicit decoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(decoder);

  ~decoder() = default;

  H3C_EXPORT buffer decode(buffer &encoded,
                           uint8_t prefix,
                           std::error_code &ec) const;

  H3C_EXPORT buffer decode(buffers &encoded,
                           uint8_t prefix,
                           std::error_code &ec) const;

private:
  logger *logger_;

  prefix_int::decoder prefix_int_;
  huffman::decoder huffman_;

  template <typename Sequence>
  buffer decode(Sequence &encoded, uint8_t prefix, std::error_code &ec) const;
};

} // namespace literal
} // namespace h3c
