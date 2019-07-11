#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/macro.hpp>
#include <bnl/base/result.hpp>
#include <bnl/base/string_view.hpp>
#include <bnl/http3/export.hpp>

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

  template <typename Sequence>
  base::result<base::string> decode(Sequence &encoded,
                                    size_t encoded_size) const;

private:
  template <typename Lookahead>
  base::result<size_t> decoded_size(const Lookahead &encoded,
                                    size_t encoded_size) const noexcept;

private:
  const log::api *logger_;
};

#define BNL_HTTP3_QPACK_HUFFMAN_DECODE_IMPL(T)                                 \
  template BNL_HTTP3_EXPORT base::result<base::string> decoder::decode<T>(     \
      T &, size_t /* NOLINT */) const

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_QPACK_HUFFMAN_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_DECL(BNL_HTTP3_QPACK_HUFFMAN_DECODE_IMPL);

} // namespace huffman
} // namespace qpack
} // namespace http3
} // namespace bnl
