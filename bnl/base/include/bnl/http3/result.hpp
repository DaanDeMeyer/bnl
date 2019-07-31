#pragma once

#include <bnl/base/result.hpp>

#include <cstdint>

namespace bnl {
namespace http3 {

enum class error : uint64_t {
  no_error = 0x0,
  internal = 0x03,
  wrong_stream = 0x0a,
  closed_critical_stream = 0x0f,
  missing_settings = 0x0012,
  unexpected_frame = 0x0013,
  malformed_frame = 0x0100,
  qpack_decompression_failed = 0x200,

  incomplete = 0x40ULL << 56U,
  idle,
  delegate,
  not_implemented,
  varint_overflow,
  malformed_header,
  stream_closed,
  invalid_handle
};

template <typename T>
using result = base::result<T, error>;

}
}
