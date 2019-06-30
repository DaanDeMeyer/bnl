#pragma once

#include <bnl/http3/export.hpp>
#include <bnl/http3/header.hpp>

#include <bnl/http3/codec/frame.hpp>
#include <bnl/http3/codec/qpack.hpp>

#include <bnl/buffer.hpp>
#include <bnl/buffers.hpp>

#include <bnl/class/macro.hpp>

#include <bnl/nothing.hpp>

#include <queue>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace headers {

class BNL_HTTP3_EXPORT encoder {
public:
  explicit encoder(const log::api *logger) noexcept;

  BNL_MOVE_ONLY(encoder);

  nothing add(header_view header, std::error_code &ec);
  nothing fin(std::error_code &ec) noexcept;

  bool finished() const noexcept;

  buffer encode(std::error_code &ec) noexcept;

private:
  const log::api *logger_;

  frame::encoder frame_;
  qpack::encoder qpack_;

  enum class state : uint8_t { idle, frame, qpack, fin, error };

  state state_ = state::idle;
  std::queue<buffer> buffers_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  BNL_MOVE_ONLY(decoder);

  bool started() const noexcept;

  bool finished() const noexcept;

  header decode(buffers &encoded, std::error_code &ec) noexcept;

private:
  const log::api *logger_;

  frame::decoder frame_;
  qpack::decoder qpack_;

  enum class state : uint8_t { frame, qpack, fin, error };

  state state_ = state::frame;
  uint64_t headers_size_ = 0;
};

} // namespace headers
} // namespace http3
} // namespace bnl
