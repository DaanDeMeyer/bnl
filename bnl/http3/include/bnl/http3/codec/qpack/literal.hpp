#pragma once

#include <bnl/http3/export.hpp>

#include <bnl/http3/codec/qpack/huffman.hpp>
#include <bnl/http3/codec/qpack/prefix_int.hpp>

#include <bnl/buffer.hpp>
#include <bnl/buffer_view.hpp>
#include <bnl/buffers.hpp>

#include <bnl/class/macro.hpp>

#include <cstddef>
#include <cstdint>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace qpack {
namespace literal {

class BNL_HTTP3_EXPORT encoder {
public:
  explicit encoder(const log::api *logger) noexcept;

  BNL_MOVE_ONLY(encoder);

  size_t encoded_size(buffer_view literal, uint8_t prefix) const noexcept;

  size_t encode(uint8_t *dest, buffer_view literal, uint8_t prefix) const
      noexcept;

  buffer encode(buffer_view literal, uint8_t prefix) const;

private:
  const log::api *logger_;

  prefix_int::encoder prefix_int_;
  huffman::encoder huffman_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  BNL_MOVE_ONLY(decoder);

  buffer decode(buffer::lookahead &encoded,
                uint8_t prefix,
                std::error_code &ec) const;

  buffer decode(buffers::lookahead &encoded,
                uint8_t prefix,
                std::error_code &ec) const;

private:
  const log::api *logger_;

  prefix_int::decoder prefix_int_;
  huffman::decoder huffman_;

  template <typename Lookahead>
  buffer decode(Lookahead &encoded, uint8_t prefix, std::error_code &ec) const;
};

} // namespace literal
} // namespace qpack
} // namespace http3
} // namespace bnl
