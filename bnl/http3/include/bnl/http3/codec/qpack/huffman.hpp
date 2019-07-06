#pragma once

#include <bnl/http3/export.hpp>

#include <bnl/buffer.hpp>
#include <bnl/buffers.hpp>
#include <bnl/string_view.hpp>

#include <bnl/class/macro.hpp>

#include <cstddef>
#include <cstdint>
#include <system_error>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace qpack {
namespace huffman {

class BNL_HTTP3_EXPORT encoder {
public:
  explicit encoder(const log::api *logger) noexcept;

  BNL_MOVE_ONLY(encoder);

  size_t encoded_size(string_view string) const noexcept;

  size_t encode(uint8_t *dest, string_view string) const noexcept;

  buffer encode(string_view string) const;

private:
  const log::api *logger_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  BNL_MOVE_ONLY(decoder);

  string decode(buffer &encoded,
                size_t encoded_size,
                std::error_code &ec) const;

  string decode(buffers &encoded,
                size_t encoded_size,
                std::error_code &ec) const;

  string decode(buffer::lookahead &encoded,
                size_t encoded_size,
                std::error_code &ec) const;

  string decode(buffers::lookahead &encoded,
                size_t encoded_size,
                std::error_code &ec) const;

private:
  const log::api *logger_;

  template <typename Lookahead>
  string decode(Lookahead &encoded,
                size_t encoded_size,
                std::error_code &ec) const;

  template <typename Lookahead>
  size_t decoded_size(const Lookahead &encoded,
                      size_t encoded_size,
                      std::error_code &ec) const noexcept;
};

} // namespace huffman
} // namespace qpack
} // namespace http3
} // namespace bnl
