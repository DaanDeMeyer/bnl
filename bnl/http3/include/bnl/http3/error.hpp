#pragma once

#include <bnl/http3/export.hpp>

#include <cstdint>
#include <system_error>

namespace bnl {
namespace http3 {

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

  // http3 library errors

  // QUIC's error codes are limited to 16 bytes which leaves all numbers larger
  // than 16 bytes for library error codes.

  varint_overflow = UINT16_MAX + 1,
  malformed_header,
  stream_closed,
  stream_exists,
  invalid_handle
};

BNL_HTTP3_EXPORT const std::error_category &
error_category() noexcept;

BNL_HTTP3_EXPORT std::error_code
make_error_code(error error) noexcept;

}
}

namespace std {

template<>
struct is_error_code_enum<bnl::http3::error> : true_type {
};

}
