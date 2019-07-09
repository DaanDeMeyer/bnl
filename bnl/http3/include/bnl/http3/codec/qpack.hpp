#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/macro.hpp>
#include <bnl/http3/codec/qpack/literal.hpp>
#include <bnl/http3/codec/qpack/prefix_int.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/header.hpp>

#include <system_error>

// https://quicwg.org/base-drafts/draft-ietf-quic-qpack.html

// This QPACK implementation only uses the static table to keep the
// implementation and usage simple. Users should advertise
// `SETTINGS_QPACK_MAX_TABLE_CAPACITY` as zero when using this implementation.

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace qpack {

class BNL_HTTP3_EXPORT encoder {
public:
  explicit encoder(const log::api *logger) noexcept;

  BNL_BASE_MOVE_ONLY(encoder);

  uint64_t count() const noexcept;

  size_t encoded_size(header_view header, std::error_code &ec) const noexcept;

  size_t encode(uint8_t *dest,
                header_view header,
                std::error_code &ec) noexcept;

  base::buffer encode(header_view header, std::error_code &ec);

private:
  enum class state { prefix, header };

  state state_ = state::prefix;
  uint64_t count_ = 0;

  prefix_int::encoder prefix_int_;
  literal::encoder literal_;

  const log::api *logger_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger);

  BNL_BASE_MOVE_ONLY(decoder);

  uint64_t count() const noexcept;

  template <typename Lookahead>
  header decode(Lookahead &encoded, std::error_code &ec);

private:
  enum class state { prefix, header };

  state state_ = state::prefix;
  uint64_t count_ = 0;

  prefix_int::decoder prefix_int_;
  literal::decoder literal_;

  const log::api *logger_;
};

#define BNL_HTTP3_QPACK_DECODE_IMPL(T)                                         \
  template BNL_HTTP3_EXPORT header                                             \
  decoder::decode<T>(T &, std::error_code &) // NOLINT

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_QPACK_DECODE_IMPL);

} // namespace qpack
} // namespace http3
} // namespace bnl
