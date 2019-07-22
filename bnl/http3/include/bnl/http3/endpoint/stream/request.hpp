#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/http3/codec/body.hpp>
#include <bnl/http3/codec/frame.hpp>
#include <bnl/http3/codec/headers.hpp>
#include <bnl/http3/event.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/header.hpp>
#include <bnl/quic/event.hpp>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace endpoint {
namespace stream {
namespace request {

class BNL_HTTP3_EXPORT sender {
public:
  sender(uint64_t id, const log::api *logger) noexcept;

  sender(sender &&other) noexcept;
  sender &operator=(sender &&other) noexcept;

  ~sender() noexcept;

  bool finished() const noexcept;

  result<quic::event> send() noexcept;

  result<void> header(header_view header);
  result<void> body(base::buffer body);

  result<void> start() noexcept;
  result<void> fin() noexcept;

  class handle {
  public:
    handle() = default;
    explicit handle(stream::request::sender *sender);

    handle(handle &&other) noexcept;
    handle &operator=(handle &&other) noexcept;

    ~handle();

    uint64_t id() const noexcept;

    result<void> header(header_view header);
    result<void> body(base::buffer body);

    result<void> start() noexcept;
    result<void> fin() noexcept;

  private:
    friend class sender;

    uint64_t id_ = UINT64_MAX;
    stream::request::sender *sender_ = nullptr;
  };

private:
  enum class state : uint8_t { headers, body, fin };

  class handle *handle_ = nullptr;

  state state_ = state::headers;

  headers::encoder headers_;
  body::encoder body_;

  uint64_t id_;
  const log::api *logger_;
};

class BNL_HTTP3_EXPORT receiver {
public:
  receiver(uint64_t id, const log::api *logger) noexcept;

  receiver(receiver &&) = default;
  receiver &operator=(receiver &&) = default;

  virtual ~receiver() noexcept;

  bool closed() const noexcept;

  bool finished() const noexcept;

  result<void> start() noexcept;

  result<void> recv(quic::data data, event::handler handler);

protected:
  virtual result<event> process(frame frame) noexcept = 0;

  const headers::decoder &headers() const noexcept;

private:
  result<event> process() noexcept;

private:
  enum class state : uint8_t { closed, headers, body, fin };

  state state_ = state::closed;
  base::buffers buffers_;
  bool fin_received_ = false;

  frame::decoder frame_;
  headers::decoder headers_;
  body::decoder body_;

  uint64_t id_;
  const log::api *logger_;
};

}
}
}
}
}
