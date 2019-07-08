#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/macro.hpp>
#include <bnl/base/nothing.hpp>
#include <bnl/http3/codec/frame.hpp>
#include <bnl/http3/export.hpp>

#include <queue>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace body {

class BNL_HTTP3_EXPORT encoder {
public:
  explicit encoder(const log::api *logger) noexcept;

  BNL_BASE_MOVE_ONLY(encoder);

  base::nothing add(base::buffer body, std::error_code &ec);
  base::nothing fin(std::error_code &ec) noexcept;

  bool finished() const noexcept;

  base::buffer encode(std::error_code &ec) noexcept;

private:
  const log::api *logger_;

  frame::encoder frame_;

  enum class state : uint8_t { frame, data, fin, error };

  state state_ = state::frame;
  bool fin_ = false;
  std::queue<base::buffer> buffers_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  BNL_BASE_MOVE_ONLY(decoder);

  bool in_progress() const noexcept;

  template <typename Sequence>
  base::buffer decode(Sequence &encoded, std::error_code &ec);

private:
  const log::api *logger_;

  frame::decoder frame_;

  enum class state : uint8_t { frame, data, error };

  state state_ = state::frame;
  uint64_t remaining_ = 0;
};

#define BNL_HTTP3_BODY_DECODE_IMPL(T)                                          \
  template BNL_HTTP3_EXPORT base::buffer decoder::decode<T>(                   \
      T &, /* NOLINT */ std::error_code &)

BNL_BASE_SEQUENCE_DECL(BNL_HTTP3_BODY_DECODE_IMPL);

} // namespace body
} // namespace http3
} // namespace bnl
