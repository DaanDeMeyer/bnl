#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/string_view.hpp>
#include <bnl/base/template.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/result.hpp>

#include <cstddef>
#include <cstdint>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace qpack {
namespace huffman {

class BNL_HTTP3_EXPORT encoder {
public:
  encoder() = default;

  encoder(encoder &&) = default;
  encoder &operator=(encoder &&) = default;

  size_t encoded_size(base::string_view string) const noexcept;

  size_t encode(uint8_t *dest, base::string_view string) const noexcept;

  base::buffer encode(base::string_view string) const;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  decoder(decoder &&) = default;
  decoder &operator=(decoder &&) = default;

  template<typename Sequence>
  result<base::string> decode(Sequence &encoded, size_t encoded_size) const;

private:
  template<typename Lookahead>
  result<size_t> decoded_size(const Lookahead &encoded,
                              size_t encoded_size) const noexcept;

private:
  const log::api *logger_;
};

#define BNL_HTTP3_QPACK_HUFFMAN_DECODE_IMPL(T)                                 \
  template BNL_HTTP3_EXPORT result<base::string> decoder::decode<T>(           \
    T &, size_t) const // NOLINT

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_QPACK_HUFFMAN_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_DECL(BNL_HTTP3_QPACK_HUFFMAN_DECODE_IMPL);

}
}
}
}
