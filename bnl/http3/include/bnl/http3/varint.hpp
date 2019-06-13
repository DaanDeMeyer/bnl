#pragma once

#include <bnl/buffer.hpp>
#include <bnl/buffers.hpp>
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

  size_t encoded_size(uint64_t varint, std::error_code &ec) const noexcept;

  size_t
  encode(uint8_t *dest, uint64_t varint, std::error_code &ec) const noexcept;

  buffer encode(uint64_t varint, std::error_code &ec) const;

private:
  const log::api *logger_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  uint64_t decode(buffer &encoded, std::error_code &ec) const noexcept;

  uint64_t decode(buffers &encoded, std::error_code &ec) const noexcept;

private:
  const log::api *logger_;

  template <typename Sequence>
  uint64_t decode(Sequence &encoded, std::error_code &ec) const noexcept;
};

} // namespace varint

} // namespace http3
} // namespace bnl
