#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/template.hpp>
#include <bnl/http3/codec/qpack/huffman.hpp>
#include <bnl/http3/codec/qpack/prefix_int.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/result.hpp>

#include <cstddef>
#include <cstdint>

namespace bnl {
namespace http3 {
namespace qpack {
namespace literal {

BNL_HTTP3_EXPORT size_t
encoded_size(base::string_view literal, uint8_t prefix) noexcept;

BNL_HTTP3_EXPORT size_t
encode(uint8_t *dest, base::string_view literal, uint8_t prefix) noexcept;

BNL_HTTP3_EXPORT base::buffer
encode(base::string_view literal, uint8_t prefix);

template<typename Sequence>
BNL_HTTP3_EXPORT result<base::string>
decode(Sequence &encoded, uint8_t prefix);

#define BNL_HTTP3_QPACK_LITERAL_DECODE_IMPL(T)                                 \
  template BNL_HTTP3_EXPORT result<base::string> decode<T>(T &, /* NOLINT */   \
                                                           uint8_t)

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_QPACK_LITERAL_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_DECL(BNL_HTTP3_QPACK_LITERAL_DECODE_IMPL);

}
}
}
}
