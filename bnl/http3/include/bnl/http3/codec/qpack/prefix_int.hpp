#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/template.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/result.hpp>

#include <cstdint>

namespace bnl {
namespace http3 {
namespace qpack {
namespace prefix_int {

BNL_HTTP3_EXPORT size_t
encoded_size(uint64_t value, uint8_t prefix) noexcept;

BNL_HTTP3_EXPORT size_t
encode(uint8_t *dest, uint64_t value, uint8_t prefix) noexcept;

BNL_HTTP3_EXPORT base::buffer
encode(uint64_t value, uint8_t prefix);

template<typename Sequence>
BNL_HTTP3_EXPORT result<uint64_t>
decode(Sequence &encoded, uint8_t prefix) noexcept;

#define BNL_HTTP3_QPACK_PREFIX_INT_DECODE_IMPL(T)                              \
  template BNL_HTTP3_EXPORT result<uint64_t> decode<T>(T & /* NOLINT */,       \
                                                       uint8_t) noexcept

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_QPACK_PREFIX_INT_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_DECL(BNL_HTTP3_QPACK_PREFIX_INT_DECODE_IMPL);

}
}
}
}
