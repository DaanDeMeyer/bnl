#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/template.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/result.hpp>

#include <cstdint>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace qpack {
namespace prefix_int {

class BNL_HTTP3_EXPORT encoder {
public:
  encoder() = default;

  encoder(encoder &&) = default;
  encoder &operator=(encoder &&) = default;

  size_t encoded_size(uint64_t value, uint8_t prefix) const noexcept;

  size_t encode(uint8_t *dest, uint64_t value, uint8_t prefix) const noexcept;

  base::buffer encode(uint64_t value, uint8_t prefix) const;
};

class BNL_HTTP3_EXPORT decoder {
public:
  decoder() = default;

  decoder(decoder &&) = default;
  decoder &operator=(decoder &&) = default;

  template<typename Sequence>
  result<uint64_t> decode(Sequence &encoded, uint8_t prefix) const noexcept;

private:
  template<typename Lookahead>
  result<uint8_t> uint8_decode(Lookahead &encoded) const noexcept;
};

#define BNL_HTTP3_QPACK_PREFIX_INT_DECODE_IMPL(T)                              \
  template BNL_HTTP3_EXPORT result<uint64_t> decoder::decode<T>(               \
    T & /* NOLINT */, uint8_t) const noexcept

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_QPACK_PREFIX_INT_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_DECL(BNL_HTTP3_QPACK_PREFIX_INT_DECODE_IMPL);

}
}
}
}
