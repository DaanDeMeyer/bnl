#pragma once

#include <h3c/endpoint/stream/body.hpp>
#include <h3c/endpoint/stream/headers.hpp>
#include <h3c/event.hpp>
#include <h3c/export.hpp>
#include <h3c/frame.hpp>
#include <h3c/header.hpp>

#include <h3c/util/class.hpp>

namespace h3c {

class logger;

namespace stream {
namespace request {

class sender;

class H3C_EXPORT handle {
public:
  handle() = default;
  explicit handle(uint64_t id,
                             stream::request::sender *ref,
                             logger *logger) noexcept;

  H3C_NO_COPY(handle);

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
  logger *logger_ = nullptr;
};

class H3C_EXPORT sender {
public:
  sender(uint64_t id, logger *logger) noexcept;

  H3C_NO_COPY(sender);

  sender(sender &&other) noexcept;
  sender &operator=(sender &&other) noexcept;

  ~sender() noexcept;

  request::handle handle() noexcept;

  bool finished() const noexcept;

  quic::data send(std::error_code &ec) noexcept;

private:
  friend request::handle;

  uint64_t id_;
  logger *logger_;

  stream::headers::encoder headers_;
  stream::body::encoder body_;

  enum state : uint8_t { headers, body, fin, error };

  state state_ = state::headers;
  request::handle *handle_ = nullptr;
};

class H3C_EXPORT receiver {
public:
  receiver(uint64_t id, logger *logger) noexcept;

  H3C_MOVE_ONLY(receiver);

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
  logger *logger_;

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
} // namespace h3c
