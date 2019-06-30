#pragma once

#include <bnl/http3/export.hpp>

#include <bnl/http3/codec/frame.hpp>

#include <bnl/buffer.hpp>
#include <bnl/buffers.hpp>

#include <bnl/nothing.hpp>

#include <bnl/class/macro.hpp>

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

  BNL_MOVE_ONLY(encoder);

  nothing add(buffer body, std::error_code &ec);
  nothing fin(std::error_code &ec) noexcept;

  bool finished() const noexcept;

  buffer encode(std::error_code &ec) noexcept;

private:
  const log::api *logger_;

  frame::encoder frame_;

  enum class state : uint8_t { frame, data, fin, error };

  state state_ = state::frame;
  bool fin_ = false;
  std::queue<buffer> buffers_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  BNL_MOVE_ONLY(decoder);

  bool in_progress() const noexcept;

  buffer decode(buffers &encoded, std::error_code &ec) noexcept;

private:
  const log::api *logger_;

  frame::decoder frame_;

  enum class state : uint8_t { frame, data, error };

  state state_ = state::frame;
  uint64_t remaining_ = 0;
};

} // namespace body
} // namespace http3
} // namespace bnl
