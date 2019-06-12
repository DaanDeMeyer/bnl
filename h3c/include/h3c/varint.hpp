#pragma once

#include <h3c/buffer.hpp>
#include <h3c/buffers.hpp>
#include <h3c/export.hpp>
#include <h3c/util/class.hpp>

#include <cstddef>
#include <cstdint>
#include <system_error>

// https://quicwg.org/base-drafts/draft-ietf-quic-transport.html#integer-encoding

namespace h3c {

class logger;

namespace varint {

static constexpr uint64_t max = (0x40ULL << 56U) - 1;

class H3C_EXPORT encoder {
public:
  explicit encoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(encoder);

  ~encoder() = default;

  size_t encoded_size(uint64_t varint, std::error_code &ec) const noexcept;

  size_t
  encode(uint8_t *dest, uint64_t varint, std::error_code &ec) const noexcept;

  buffer encode(uint64_t varint, std::error_code &ec) const;

private:
  logger *logger_;
};

class H3C_EXPORT decoder {
public:
  explicit decoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(decoder);

  ~decoder() = default;

  uint64_t decode(buffer &encoded, std::error_code &ec) const noexcept;

  uint64_t decode(buffers &encoded, std::error_code &ec) const noexcept;

private:
  logger *logger_;

  template <typename Sequence>
  uint64_t decode(Sequence &encoded, std::error_code &ec) const noexcept;
};

} // namespace varint

} // namespace h3c
