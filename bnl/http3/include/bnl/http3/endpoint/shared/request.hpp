#pragma once

#include <bnl/http3/endpoint/handle.hpp>

#include <bnl/http3/event.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/header.hpp>

#include <bnl/http3/codec/body.hpp>
#include <bnl/http3/codec/frame.hpp>
#include <bnl/http3/codec/headers.hpp>

#include <bnl/quic/event.hpp>

#include <bnl/buffer.hpp>
#include <bnl/buffers.hpp>

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

  BNL_NO_COPY(sender);
  BNL_CUSTOM_MOVE(sender);

  ~sender() noexcept;

  endpoint::handle handle() noexcept;

  bool finished() const noexcept;

  quic::data send(std::error_code &ec) noexcept;

private:
  friend endpoint::handle;

  uint64_t id_;
  const log::api *logger_;

  headers::encoder headers_;
  body::encoder body_;

  enum state : uint8_t { headers, body, fin, error };

  state state_ = state::headers;
  endpoint::handle *handle_ = nullptr;
};

class BNL_HTTP3_EXPORT receiver {
public:
  receiver(uint64_t id, const log::api *logger) noexcept;

  BNL_NO_COPY(receiver);
  BNL_DEFAULT_MOVE(receiver);

  virtual ~receiver() noexcept;

  bool closed() const noexcept;

  bool finished() const noexcept;

  void start(std::error_code &ec) noexcept;

  void recv(quic::data data, event::handler handler, std::error_code &ec);

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
