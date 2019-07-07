#pragma once

#include <bnl/http3/export.hpp>

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/macro.hpp>
#include <bnl/base/string_view.hpp>

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

  BNL_BASE_MOVE_ONLY(encoder);

  size_t encoded_size(base::string_view string) const noexcept;

  size_t encode(uint8_t *dest, base::string_view string) const noexcept;

  base::buffer encode(base::string_view string) const;

private:
  const log::api *logger_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  BNL_BASE_MOVE_ONLY(decoder);

  base::string decode(base::buffer &encoded,
                      size_t encoded_size,
                      std::error_code &ec) const;

  base::string decode(base::buffers &encoded,
                      size_t encoded_size,
                      std::error_code &ec) const;

  base::string decode(base::buffer::lookahead &encoded,
                      size_t encoded_size,
                      std::error_code &ec) const;

  base::string decode(base::buffers::lookahead &encoded,
                      size_t encoded_size,
                      std::error_code &ec) const;

private:
  const log::api *logger_;

  template <typename Lookahead>
  base::string decode(Lookahead &encoded,
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
