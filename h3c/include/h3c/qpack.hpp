#pragma once

#include <h3c/export.hpp>
#include <h3c/header.hpp>
#include <h3c/literal.hpp>
#include <h3c/prefix_int.hpp>
#include <h3c/util/class.hpp>

#include <system_error>

// https://quicwg.org/base-drafts/draft-ietf-quic-qpack.html

// This QPACK implementation only uses the static table to keep the
// implementation and usage simple. Users should advertise
// `SETTINGS_QPACK_MAX_TABLE_CAPACITY` as zero when using this implementation.

namespace h3c {

class logger;

namespace qpack {

class H3C_EXPORT encoder {
public:
  explicit encoder(const logger *logger) noexcept;

  H3C_MOVE_ONLY(encoder);

  ~encoder() = default;

  uint64_t count() const noexcept;

  size_t encoded_size(header_view header, std::error_code &ec) const noexcept;

  size_t
  encode(uint8_t *dest, header_view header, std::error_code &ec) noexcept;

  buffer encode(header_view header, std::error_code &ec);

private:
  const logger *logger_;

  prefix_int::encoder prefix_int_;
  literal::encoder literal_;

  enum class state { prefix, header };

  state state_ = state::prefix;
  uint64_t count_ = 0;
};

class H3C_EXPORT decoder {
public:
  explicit decoder(const logger *logger);

  H3C_MOVE_ONLY(decoder);

  ~decoder() = default;

  uint64_t count() const noexcept;

  header decode(buffer &encoded, std::error_code &ec);

  header decode(buffers &encoded, std::error_code &ec);

private:
  const logger *logger_;

  prefix_int::decoder prefix_int_;
  literal::decoder literal_;

  enum class state { prefix, header };

  state state_ = state::prefix;
  uint64_t count_ = 0;

  template <typename Sequence>
  header decode(Sequence &encoded, std::error_code &ec);
};

} // namespace qpack
} // namespace h3c
