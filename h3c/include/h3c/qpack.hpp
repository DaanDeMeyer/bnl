#pragma once

#include <h3c/export.hpp>
#include <h3c/header.hpp>
#include <h3c/literal.hpp>
#include <h3c/prefix_int.hpp>
#include <h3c/util/class.hpp>

#include <memory>
#include <system_error>

// https://quicwg.org/base-drafts/draft-ietf-quic-qpack.html

// This QPACK implementation only uses the static table to keep the
// implementation and usage simple. Users should advertise
// `SETTINGS_QPACK_MAX_TABLE_CAPACITY` as zero when using this implementation.

namespace h3c {

class logger;

namespace qpack {

class encoder {
public:
  H3C_EXPORT explicit encoder(logger *logger) noexcept;

  H3C_MOVE_ONLY(encoder)

  H3C_EXPORT size_t prefix_encoded_size() const noexcept;

  H3C_EXPORT size_t prefix_encode(uint8_t *dest) const noexcept;

  H3C_EXPORT buffer prefix_encode() const;

  H3C_EXPORT size_t encoded_size(const header &header,
                                 std::error_code &ec) const noexcept;

  H3C_EXPORT size_t encode(uint8_t *dest,
                           const header &header,
                           std::error_code &ec) const noexcept;

  H3C_EXPORT buffer encode(const header &header, std::error_code &ec) const;

private:
  logger *logger_;

  prefix_int::encoder prefix_int_;
  literal::encoder literal_;
};

class decoder {
public:
  H3C_EXPORT explicit decoder(logger *logger);

  H3C_MOVE_ONLY(decoder)

  H3C_EXPORT void
  prefix_decode(buffer &encoded, std::error_code &ec) const noexcept;

  H3C_EXPORT header decode(buffer &encoded, std::error_code &ec) const;

private:
  logger *logger_;

  prefix_int::decoder prefix_int_;
  literal::decoder literal_;
};

} // namespace qpack
} // namespace h3c
