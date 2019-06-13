#pragma once

#include <bnl/http3/export.hpp>

#include <bnl/http3/codec/huffman.hpp>
#include <bnl/http3/codec/prefix_int.hpp>

#include <bnl/buffer.hpp>
#include <bnl/buffers.hpp>

#include <cstddef>
#include <cstdint>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace literal {

class BNL_HTTP3_EXPORT encoder {
public:
  explicit encoder(const log::api *logger) noexcept;

  size_t encoded_size(buffer_view literal, uint8_t prefix) const noexcept;

  size_t
  encode(uint8_t *dest, buffer_view literal, uint8_t prefix) const noexcept;

  buffer encode(buffer_view literal, uint8_t prefix) const;

private:
  const log::api *logger_;

  prefix_int::encoder prefix_int_;
  huffman::encoder huffman_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  buffer decode(buffer &encoded, uint8_t prefix, std::error_code &ec) const;

  buffer decode(buffers &encoded, uint8_t prefix, std::error_code &ec) const;

private:
  const log::api *logger_;

  prefix_int::decoder prefix_int_;
  huffman::decoder huffman_;

  template <typename Sequence>
  buffer decode(Sequence &encoded, uint8_t prefix, std::error_code &ec) const;
};

} // namespace literal
} // namespace http3
} // namespace bnl
