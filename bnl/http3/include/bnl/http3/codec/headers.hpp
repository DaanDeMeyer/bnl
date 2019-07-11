#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/macro.hpp>
#include <bnl/http3/codec/frame.hpp>
#include <bnl/http3/codec/qpack.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/header.hpp>

#include <queue>

namespace bnl {

namespace log {
class api;
} // namespace log

namespace http3 {
namespace headers {

class BNL_HTTP3_EXPORT encoder {
public:
  explicit encoder(const log::api *logger) noexcept;

  BNL_BASE_MOVE_ONLY(encoder);

  std::error_code add(header_view header);
  std::error_code fin() noexcept;

  bool finished() const noexcept;

  base::result<base::buffer> encode() noexcept;

private:
  enum class state : uint8_t { idle, frame, qpack, fin };

  state state_ = state::idle;
  std::queue<base::buffer> buffers_;

  frame::encoder frame_;
  qpack::encoder qpack_;

  const log::api *logger_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  BNL_BASE_MOVE_ONLY(decoder);

  bool started() const noexcept;

  bool finished() const noexcept;

  template <typename Sequence>
  base::result<header> decode(Sequence &encoded);

private:
  enum class state : uint8_t { frame, qpack, fin };

  state state_ = state::frame;
  uint64_t headers_size_ = 0;

  frame::decoder frame_;
  qpack::decoder qpack_;

  const log::api *logger_;
};

#define BNL_HTTP3_HEADERS_DECODE_IMPL(T)                                       \
  template BNL_HTTP3_EXPORT base::result<header> decoder::decode<T>(           \
      T &) // NOLINT

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_HEADERS_DECODE_IMPL);

} // namespace headers
} // namespace http3
} // namespace bnl
