#pragma once

#include <bnl/http3/event.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/header.hpp>

#include <bnl/http3/codec/body.hpp>
#include <bnl/http3/codec/frame.hpp>
#include <bnl/http3/codec/headers.hpp>

#include <bnl/quic/event.hpp>

#include <bnl/buffer.hpp>
#include <bnl/buffers.hpp>
#include <bnl/nothing.hpp>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace endpoint {
namespace shared {
namespace request {

class BNL_HTTP3_EXPORT sender {
public:
  sender(uint64_t id, const log::api *logger) noexcept;

  BNL_MOVE_ONLY(sender);

  bool finished() const noexcept;

  quic::event send(std::error_code &ec) noexcept;

  nothing header(header_view header, std::error_code &ec);
  nothing body(buffer body, std::error_code &ec);

  nothing start(std::error_code &ec) noexcept;
  nothing fin(std::error_code &ec) noexcept;

private:
  uint64_t id_;
  const log::api *logger_;

  headers::encoder headers_;
  body::encoder body_;

  enum class state : uint8_t { headers, body, fin, error };

  state state_ = state::headers;
};

class BNL_HTTP3_EXPORT receiver {
public:
  receiver(uint64_t id, const log::api *logger) noexcept;

  BNL_NO_COPY(receiver);
  BNL_DEFAULT_MOVE(receiver);

  virtual ~receiver() noexcept;

  bool closed() const noexcept;

  bool finished() const noexcept;

  nothing start(std::error_code &ec) noexcept;

  nothing recv(quic::event event, event::handler handler, std::error_code &ec);

protected:
  virtual event process(frame frame, std::error_code &ec) noexcept = 0;

  const headers::decoder &headers() const noexcept;

private:
  event process(std::error_code &ec) noexcept;

  uint64_t id_;
  const log::api *logger_;

  frame::decoder frame_;
  headers::decoder headers_;
  body::decoder body_;

  enum class state : uint8_t { closed, headers, body, fin, error };

  state state_ = state::closed;

  buffers buffers_;
  bool fin_received_ = false;
};

} // namespace request
} // namespace shared
} // namespace endpoint
} // namespace http3
} // namespace bnl
