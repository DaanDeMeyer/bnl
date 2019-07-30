#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/template.hpp>
#include <bnl/http3/codec/qpack/literal.hpp>
#include <bnl/http3/codec/qpack/prefix_int.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/header.hpp>

// https://quicwg.org/base-drafts/draft-ietf-quic-qpack.html

// This QPACK implementation only uses the static table to keep the
// implementation and usage simple. Users should advertise
// `SETTINGS_QPACK_MAX_TABLE_CAPACITY` as zero when using this implementation.

namespace bnl {
namespace http3 {
namespace qpack {

class BNL_HTTP3_EXPORT encoder {
public:
  encoder() = default;

  encoder(encoder &&) = default;
  encoder &operator=(encoder &&) = default;

  uint64_t count() const noexcept;

  result<size_t> encoded_size(header_view header) const noexcept;

  result<size_t> encode(uint8_t *dest, header_view header) noexcept;

  result<base::buffer> encode(header_view header);

private:
  enum class state { prefix, header };

  state state_ = state::prefix;
  uint64_t count_ = 0;
};

class BNL_HTTP3_EXPORT decoder {
public:
  decoder() = default;

  decoder(decoder &&) = default;
  decoder &operator=(decoder &&) = default;

  uint64_t count() const noexcept;

  template<typename Lookahead>
  result<header> decode(Lookahead &encoded);

private:
  enum class state { prefix, header };

  state state_ = state::prefix;
  uint64_t count_ = 0;
};

#define BNL_HTTP3_QPACK_DECODE_IMPL(T)                                         \
  template BNL_HTTP3_EXPORT result<header> decoder::decode<T>(T &) // NOLINT

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_QPACK_DECODE_IMPL);

}
}
}
