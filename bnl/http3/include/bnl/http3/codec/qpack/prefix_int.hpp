#pragma once

#include <bnl/http3/export.hpp>

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/macro.hpp>

#include <cstdint>
#include <system_error>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace qpack {
namespace prefix_int {

class BNL_HTTP3_EXPORT encoder {
public:
  explicit encoder(const log::api *logger) noexcept;

  BNL_BASE_MOVE_ONLY(encoder);

  size_t encoded_size(uint64_t value, uint8_t prefix) const noexcept;

  size_t encode(uint8_t *dest, uint64_t value, uint8_t prefix) const noexcept;

  base::buffer encode(uint64_t value, uint8_t prefix) const;

private:
  const log::api *logger_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger);

  BNL_BASE_MOVE_ONLY(decoder);

  uint64_t decode(base::buffer::lookahead &encoded,
                  uint8_t prefix,
                  std::error_code &ec) const noexcept;

  uint64_t decode(base::buffers::lookahead &encoded,
                  uint8_t prefix,
                  std::error_code &ec) const noexcept;

private:
  const log::api *logger_;

  template <typename Lookahead>
  uint64_t decode(Lookahead &encoded, uint8_t prefix, std::error_code &ec) const
      noexcept;

  template <typename Lookahead>
  uint8_t uint8_decode(Lookahead &encoded, std::error_code &ec) const noexcept;
};

} // namespace prefix_int
} // namespace qpack
} // namespace http3
} // namespace bnl
