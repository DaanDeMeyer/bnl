#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/result.hpp>
#include <bnl/base/template.hpp>
#include <bnl/http3/export.hpp>

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

  encoder(encoder &&other) = default;
  encoder &operator=(encoder &&other) = default;

  size_t encoded_size(uint64_t value, uint8_t prefix) const noexcept;

  size_t encode(uint8_t *dest, uint64_t value, uint8_t prefix) const noexcept;

  base::buffer encode(uint64_t value, uint8_t prefix) const;

private:
  const log::api *logger_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger);

  decoder(decoder &&other) = default;
  decoder &operator=(decoder &&other) = default;

  template<typename Sequence>
  base::result<uint64_t> decode(Sequence &encoded, uint8_t prefix) const
    noexcept;

private:
  template<typename Lookahead>
  base::result<uint8_t> uint8_decode(Lookahead &encoded) const noexcept;

private:
  const log::api *logger_;
};

#define BNL_HTTP3_QPACK_PREFIX_INT_DECODE_IMPL(T)                              \
  template BNL_HTTP3_EXPORT base::result<uint64_t> decoder::decode<T>(         \
    T &, uint8_t) const noexcept // NOLINT

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_QPACK_PREFIX_INT_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_DECL(BNL_HTTP3_QPACK_PREFIX_INT_DECODE_IMPL);

} // namespace prefix_int
} // namespace qpack
} // namespace http3
} // namespace bnl
