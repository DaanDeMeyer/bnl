#pragma once

#include <h3c/export.hpp>

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
  H3C_EXPORT explicit encoder(const logger *logger) noexcept;

  encoder(const encoder &) = delete;
  encoder &operator=(const encoder &) = delete;

  encoder(encoder &&) = default;
  encoder &operator=(encoder &&) = default;

  ~encoder() = default;

  H3C_EXPORT size_t encoded_size(uint64_t varint) const noexcept;

  H3C_EXPORT std::error_code encode(uint8_t *dest,
                                    size_t size,
                                    uint64_t varint,
                                    size_t *encoded_size) const noexcept;

private:
  const logger *logger;
};

class decoder {
public:
  H3C_EXPORT explicit decoder(const logger *logger) noexcept;

  decoder(const decoder &) = delete;
  decoder &operator=(const decoder &) = delete;

  decoder(decoder &&) = default;
  decoder &operator=(decoder &&) = default;

  ~decoder() = default;

  H3C_EXPORT std::error_code decode(const uint8_t *src,
                                    size_t size,
                                    uint64_t *varint,
                                    size_t *encoded_size) const noexcept;

private:
  const logger *logger;
};

} // namespace varint

} // namespace h3c
