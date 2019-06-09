#pragma once

#include <h3c/buffer.hpp>
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

class encoder {
public:
  H3C_EXPORT explicit encoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(encoder);

  ~encoder() = default;

  H3C_EXPORT size_t encoded_size(uint64_t varint, std::error_code &ec) const
      noexcept;

  H3C_EXPORT size_t encode(uint8_t *dest,
                           uint64_t varint,
                           std::error_code &ec) const noexcept;

  H3C_EXPORT buffer encode(uint64_t varint, std::error_code &ec) const;

private:
  logger *logger_;
};

class decoder {
public:
  H3C_EXPORT explicit decoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(decoder);

  ~decoder() = default;

  H3C_EXPORT uint64_t decode(buffer &encoded, std::error_code &ec) const
      noexcept;

private:
  logger *logger_;
};

} // namespace varint

} // namespace h3c
