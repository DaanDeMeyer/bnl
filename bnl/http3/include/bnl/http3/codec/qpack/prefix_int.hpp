#pragma once

#include <bnl/http3/export.hpp>

#include <bnl/buffer.hpp>
#include <bnl/buffers.hpp>

#include <bnl/class/macro.hpp>

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

  BNL_MOVE_ONLY(encoder);

  size_t encoded_size(uint64_t value, uint8_t prefix) const noexcept;

  size_t encode(uint8_t *dest, uint64_t value, uint8_t prefix) const noexcept;

  buffer encode(uint64_t value, uint8_t prefix) const;

private:
  const log::api *logger_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger);

  BNL_MOVE_ONLY(decoder);

  uint64_t decode(buffer_view &encoded,
                  uint8_t prefix,
                  std::error_code &ec) const noexcept;

  uint64_t decode(buffers_view &encoded,
                  uint8_t prefix,
                  std::error_code &ec) const noexcept;

private:
  const log::api *logger_;

  template <typename View>
  uint64_t decode(View &encoded, uint8_t prefix, std::error_code &ec) const
      noexcept;

  template <typename View>
  uint8_t uint8_decode(View &encoded, std::error_code &ec) const noexcept;
};

} // namespace prefix_int
} // namespace qpack
} // namespace http3
} // namespace bnl
