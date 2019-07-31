#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/template.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/result.hpp>

#include <cstddef>
#include <cstdint>

// https://quicwg.org/base-drafts/draft-ietf-quic-transport.html#integer-encoding

namespace bnl {
namespace http3 {
namespace varint {

static constexpr uint64_t max = (0x40ULL << 56U) - 1;

BNL_HTTP3_EXPORT result<size_t>
encoded_size(uint64_t varint) noexcept;

BNL_HTTP3_EXPORT result<size_t>
encode(uint8_t *dest, uint64_t varint) noexcept;

BNL_HTTP3_EXPORT result<base::buffer>
encode(uint64_t varint);

template<typename Sequence>
BNL_HTTP3_EXPORT result<uint64_t>
decode(Sequence &encoded) noexcept;

#define BNL_HTTP3_VARINT_DECODE_IMPL(T)                                        \
  template BNL_HTTP3_EXPORT result<uint64_t> decode<T>(                        \
    T &) /* NOLINT */ noexcept

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_VARINT_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_DECL(BNL_HTTP3_VARINT_DECODE_IMPL);

}

}
}
