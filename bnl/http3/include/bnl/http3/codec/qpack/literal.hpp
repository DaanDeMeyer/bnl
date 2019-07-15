#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/template.hpp>
#include <bnl/http3/codec/qpack/huffman.hpp>
#include <bnl/http3/codec/qpack/prefix_int.hpp>
#include <bnl/http3/export.hpp>

#include <cstddef>
#include <cstdint>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace qpack {
namespace literal {

class BNL_HTTP3_EXPORT encoder
{
public:
  explicit encoder(const log::api* logger) noexcept;

  encoder(encoder&& other) = default;
  encoder& operator=(encoder&& other) = default;

  size_t encoded_size(base::string_view literal, uint8_t prefix) const noexcept;

  size_t encode(uint8_t* dest, base::string_view literal, uint8_t prefix) const
    noexcept;

  base::buffer encode(base::string_view literal, uint8_t prefix) const;

private:
  prefix_int::encoder prefix_int_;
  huffman::encoder huffman_;

  const log::api* logger_;
};

class BNL_HTTP3_EXPORT decoder
{
public:
  explicit decoder(const log::api* logger) noexcept;

  decoder(decoder&& other) = default;
  decoder& operator=(decoder&& other) = default;

  template<typename Sequence>
  base::result<base::string> decode(Sequence& encoded, uint8_t prefix) const;

private:
  prefix_int::decoder prefix_int_;
  huffman::decoder huffman_;

  const log::api* logger_;
};

#define BNL_HTTP3_QPACK_LITERAL_DECODE_IMPL(T)                                 \
  template BNL_HTTP3_EXPORT base::result<base::string> decoder::decode<T>(     \
    T&, uint8_t) const // NOLINT

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_QPACK_LITERAL_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_DECL(BNL_HTTP3_QPACK_LITERAL_DECODE_IMPL);

} // namespace literal
} // namespace qpack
} // namespace http3
} // namespace bnl
