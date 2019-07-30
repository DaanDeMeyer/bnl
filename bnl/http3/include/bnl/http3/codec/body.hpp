#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/template.hpp>
#include <bnl/http3/codec/frame.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/result.hpp>

namespace bnl {
namespace http3 {
namespace body {

class BNL_HTTP3_EXPORT encoder {
public:
  encoder() = default;

  encoder(encoder &&) = default;
  encoder &operator=(encoder &&) = default;

  result<void> add(base::buffer body);
  result<void> fin() noexcept;

  bool finished() const noexcept;

  result<base::buffer> encode() noexcept;

private:
  enum class state : uint8_t { frame, data, fin };

  state state_ = state::frame;
  bool fin_ = false;
  base::buffers buffers_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  decoder() = default;

  decoder(decoder &&) = default;
  decoder &operator=(decoder &&) = default;

  bool in_progress() const noexcept;

  template<typename Sequence>
  result<base::buffer> decode(Sequence &encoded);

private:
  enum class state : uint8_t { frame, data };

  state state_ = state::frame;
  uint64_t remaining_ = 0;
};

#define BNL_HTTP3_BODY_DECODE_IMPL(T)                                          \
  template BNL_HTTP3_EXPORT result<base::buffer> decoder::decode<T>(           \
    T &) // NOLINT

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_BODY_DECODE_IMPL);

}
}
}
