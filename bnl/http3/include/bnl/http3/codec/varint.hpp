#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/macro.hpp>
#include <bnl/http3/export.hpp>

#include <cstddef>
#include <cstdint>
#include <system_error>

// https://quicwg.org/base-drafts/draft-ietf-quic-transport.html#integer-encoding

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace varint {

static constexpr uint64_t max = (0x40ULL << 56U) - 1;

class BNL_HTTP3_EXPORT encoder {
public:
  explicit encoder(const log::api *logger) noexcept;

  BNL_BASE_MOVE_ONLY(encoder);

  size_t encoded_size(uint64_t varint, std::error_code &ec) const noexcept;

  size_t encode(uint8_t *dest, uint64_t varint, std::error_code &ec) const
      noexcept;

  base::buffer encode(uint64_t varint, std::error_code &ec) const;

private:
  const log::api *logger_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  BNL_BASE_MOVE_ONLY(decoder);

  template <typename Sequence>
  uint64_t decode(Sequence &encoded, std::error_code &ec) const noexcept;

private:
  const log::api *logger_;
};

#define BNL_HTTP3_VARINT_DECODE_IMPL(T)                                        \
  template BNL_HTTP3_EXPORT uint64_t                                           \
  decoder::decode<T>(T &, std::error_code &) /* NOLINT */ const noexcept

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_VARINT_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_DECL(BNL_HTTP3_VARINT_DECODE_IMPL);

} // namespace varint

} // namespace http3
} // namespace bnl
