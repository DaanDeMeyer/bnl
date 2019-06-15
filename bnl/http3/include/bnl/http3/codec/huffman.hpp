#pragma once

#include <bnl/http3/export.hpp>

#include <bnl/buffer.hpp>
#include <bnl/buffers.hpp>

#include <bnl/class/macro.hpp>

#include <cstddef>
#include <cstdint>
#include <system_error>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace huffman {

class BNL_HTTP3_EXPORT encoder {
public:
  explicit encoder(const log::api *logger) noexcept;

  BNL_MOVE_ONLY(encoder);

  size_t encoded_size(buffer_view string) const noexcept;

  size_t encode(uint8_t *dest, buffer_view string) const noexcept;

  buffer encode(buffer_view string) const;

private:
  const log::api *logger_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  BNL_MOVE_ONLY(decoder);

  buffer
  decode(buffer &encoded, size_t encoded_size, std::error_code &ec) const;

  buffer
  decode(buffers &encoded, size_t encoded_size, std::error_code &ec) const;

private:
  const log::api *logger_;

  template <typename Sequence>
  buffer
  decode(Sequence &encoded, size_t encoded_size, std::error_code &ec) const;

  template <typename Sequence>
  size_t decoded_size(Sequence &encoded,
                      size_t encoded_size,
                      std::error_code &ec) const noexcept;
};

} // namespace huffman
} // namespace http3
} // namespace bnl
