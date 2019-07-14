#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/macro.hpp>
#include <bnl/base/result.hpp>
#include <bnl/http3/codec/frame.hpp>
#include <bnl/http3/export.hpp>

#include <queue>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace body {

class BNL_HTTP3_EXPORT encoder
{
public:
  explicit encoder(const log::api* logger) noexcept;

  BNL_BASE_MOVE_ONLY(encoder);

  std::error_code add(base::buffer body);
  std::error_code fin() noexcept;

  bool finished() const noexcept;

  base::result<base::buffer> encode() noexcept;

private:
  enum class state : uint8_t
  {
    frame,
    data,
    fin
  };

  state state_ = state::frame;
  bool fin_ = false;
  std::queue<base::buffer> buffers_;

  frame::encoder frame_;

  const log::api* logger_;
};

class BNL_HTTP3_EXPORT decoder
{
public:
  explicit decoder(const log::api* logger) noexcept;

  BNL_BASE_MOVE_ONLY(decoder);

  bool in_progress() const noexcept;

  template<typename Sequence>
  base::result<base::buffer> decode(Sequence& encoded);

private:
  enum class state : uint8_t
  {
    frame,
    data
  };

  state state_ = state::frame;
  uint64_t remaining_ = 0;

  frame::decoder frame_;

  const log::api* logger_;
};

#define BNL_HTTP3_BODY_DECODE_IMPL(T)                                          \
  template BNL_HTTP3_EXPORT base::result<base::buffer> decoder::decode<T>(     \
    T&) // NOLINT

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_BODY_DECODE_IMPL);

} // namespace body
} // namespace http3
} // namespace bnl
