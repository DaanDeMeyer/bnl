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

class BNL_HTTP3_EXPORT encoder {
public:
  encoder() = default;

  encoder(encoder &&) = default;
  encoder &operator=(encoder &&) = default;

  size_t encoded_size(base::string_view literal, uint8_t prefix) const noexcept;

  size_t encode(uint8_t *dest, base::string_view literal, uint8_t prefix) const
    noexcept;

  base::buffer encode(base::string_view literal, uint8_t prefix) const;

private:
  prefix_int::encoder prefix_int_;
  huffman::encoder huffman_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  decoder(decoder &&) = default;
  decoder &operator=(decoder &&) = default;

  template<typename Sequence>
  result<base::string> decode(Sequence &encoded, uint8_t prefix) const;

private:
  prefix_int::decoder prefix_int_;
  huffman::decoder huffman_;
};

#define BNL_HTTP3_QPACK_LITERAL_DECODE_IMPL(T)                                 \
  template BNL_HTTP3_EXPORT result<base::string> decoder::decode<T>(     \
    T &, uint8_t) const // NOLINT

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_QPACK_LITERAL_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_DECL(BNL_HTTP3_QPACK_LITERAL_DECODE_IMPL);

}
}
}
}
