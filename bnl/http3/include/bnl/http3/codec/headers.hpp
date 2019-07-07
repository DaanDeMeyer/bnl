#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/base/macro.hpp>
#include <bnl/base/nothing.hpp>
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

  base::nothing add(header_view header, std::error_code &ec);
  base::nothing fin(std::error_code &ec) noexcept;

  bool finished() const noexcept;

  base::buffer encode(std::error_code &ec) noexcept;

private:
  const log::api *logger_;

  frame::encoder frame_;
  qpack::encoder qpack_;

  enum class state : uint8_t { idle, frame, qpack, fin, error };

  state state_ = state::idle;
  std::queue<base::buffer> buffers_;
};

class BNL_HTTP3_EXPORT decoder {
public:
  explicit decoder(const log::api *logger) noexcept;

  BNL_BASE_MOVE_ONLY(decoder);

  bool started() const noexcept;

  bool finished() const noexcept;

  header decode(base::buffers &encoded, std::error_code &ec) noexcept;

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
