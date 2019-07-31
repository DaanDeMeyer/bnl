#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/template.hpp>
#include <bnl/http3/codec/frame.hpp>
#include <bnl/http3/codec/qpack.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/header.hpp>
#include <bnl/http3/result.hpp>

namespace bnl {
namespace http3 {
namespace headers {

class BNL_HTTP3_EXPORT encoder {
public:
  encoder() = default;

  encoder(encoder &&) = default;
  encoder &operator=(encoder &&) = default;

  result<void> add(header_view header);
  result<void> fin() noexcept;

  bool finished() const noexcept;

  result<base::buffer> encode() noexcept;

private:
  enum class state : uint8_t { idle, frame, qpack, fin };

  state state_ = state::idle;
  base::buffers buffers_;

  qpack::encoder qpack_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  decoder() = default;

  decoder(decoder &&) = default;
  decoder &operator=(decoder &&) = default;

  bool started() const noexcept;

  bool finished() const noexcept;

  template<typename Sequence>
  result<header> decode(Sequence &encoded);

private:
  enum class state : uint8_t { frame, qpack, fin };

  state state_ = state::frame;
  uint64_t headers_size_ = 0;

  qpack::decoder qpack_;
};

#define BNL_HTTP3_HEADERS_DECODE_IMPL(T)                                       \
  template BNL_HTTP3_EXPORT result<header> decoder::decode<T>(T &) // NOLINT

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_HEADERS_DECODE_IMPL);

}
}
}
