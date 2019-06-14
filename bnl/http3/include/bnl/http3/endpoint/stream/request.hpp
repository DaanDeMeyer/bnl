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

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace stream {
namespace request {

class sender;

class BNL_HTTP3_EXPORT handle {
public:
  handle() = default;
  explicit handle(uint64_t id,
                  stream::request::sender *ref,
                  const log::api *logger) noexcept;

  handle(const handle &) = delete;
  handle &operator=(const handle &) = delete;

  handle(handle &&other) noexcept;
  handle &operator=(handle &&other) noexcept;

  ~handle() noexcept;

  bool valid() const noexcept;

  uint64_t id() const noexcept;

  void header(header_view header, std::error_code &ec);
  void body(buffer body, std::error_code &ec);

  void start(std::error_code &ec) noexcept;
  void fin(std::error_code &ec) noexcept;

private:
  friend request::sender;

  uint64_t id_ = UINT64_MAX;
  request::sender *ref_ = nullptr;
  const log::api *logger_ = nullptr;
};

class BNL_HTTP3_EXPORT sender {
public:
  sender(uint64_t id, const log::api *logger) noexcept;

  sender(const sender &) = delete;
  sender &operator=(const sender &) = delete;

  sender(sender &&other) noexcept;
  sender &operator=(sender &&other) noexcept;

  ~sender() noexcept;

  request::handle handle() noexcept;

  bool finished() const noexcept;

  quic::event send(std::error_code &ec) noexcept;

private:
  friend request::handle;

  uint64_t id_;
  const log::api *logger_;

  stream::headers::encoder headers_;
  stream::body::encoder body_;

  enum state : uint8_t { headers, body, fin, error };

  state state_ = state::headers;
  request::handle *handle_ = nullptr;
};

class BNL_HTTP3_EXPORT receiver {
public:
  receiver(uint64_t id, const log::api *logger) noexcept;

  receiver(const receiver &) = default;
  receiver &operator=(const receiver &) = default;

  receiver(receiver &&) = default;
  receiver &operator=(receiver &&) = default;

  virtual ~receiver() noexcept;

  bool closed() const noexcept;

  bool finished() const noexcept;

  void start(std::error_code &ec) noexcept;

  void
  recv(quic::event quic, http3::event::handler handler, std::error_code &ec);

protected:
  virtual event process(frame frame, std::error_code &ec) noexcept = 0;

  const headers::decoder &headers() const noexcept;

private:
  event process(std::error_code &ec) noexcept;

  uint64_t id_;
  const log::api *logger_;

  frame::decoder frame_;
  stream::headers::decoder headers_;
  stream::body::decoder body_;

  enum class state : uint8_t { closed, headers, body, fin, error };

  state state_ = state::closed;

  buffers buffers_;
  bool fin_received_ = false;
};

} // namespace request
} // namespace stream
} // namespace http3
} // namespace bnl
