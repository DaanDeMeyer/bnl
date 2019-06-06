#pragma once

#include <h3c/export.hpp>

#include <cstdint>
#include <system_error>

namespace h3c {

enum class error : uint32_t {
  // HTTP/3 connection errors

  internal_error = 0x03,
  wrong_stream = 0x0a,
  closed_critical_stream = 0x0f,
  missing_settings = 0x0012,
  unexpected_frame = 0x0013,
  malformed_frame = 0x0100,
  qpack_decompression_failed = 0x200,

  // HTTP/3 stream errors

  // h3c library errors

  // QUIC's error codes are limited to 16 bytes which leaves all numbers larger
  // than 16 bytes for library error codes.

  not_implemented = UINT16_MAX + 1,
  incomplete,
  varint_overflow,
  setting_overflow,
  malformed_header,
  stream_closed
};

H3C_EXPORT const std::error_category &error_category() noexcept;

H3C_EXPORT std::error_code make_error_code(error error) noexcept;

} // namespace h3c

namespace std {

template <> struct is_error_code_enum<h3c::error> : true_type {};

} // namespace std
