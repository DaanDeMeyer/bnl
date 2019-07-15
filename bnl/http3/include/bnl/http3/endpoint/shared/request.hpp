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
namespace shared {
namespace request {

class BNL_HTTP3_EXPORT sender {
public:
  sender(uint64_t id, const log::api *logger) noexcept;

  sender(sender &&other) = default;
  sender &operator=(sender &&other) = default;

  bool finished() const noexcept;

  base::result<quic::event> send() noexcept;

  std::error_code header(header_view header);
  std::error_code body(base::buffer body);

  std::error_code start() noexcept;
  std::error_code fin() noexcept;

private:
  enum class state : uint8_t { headers, body, fin };

  state state_ = state::headers;

  headers::encoder headers_;
  body::encoder body_;

  uint64_t id_;
  const log::api *logger_;
};

class BNL_HTTP3_EXPORT receiver {
public:
  receiver(uint64_t id, const log::api *logger) noexcept;

  receiver(receiver &&other) = default;
  receiver &operator=(receiver &&other) = default;

  virtual ~receiver() noexcept;

  bool closed() const noexcept;

  bool finished() const noexcept;

  std::error_code start() noexcept;

  std::error_code recv(quic::event event, event::handler handler);

protected:
  virtual base::result<event> process(frame frame) noexcept = 0;

  const headers::decoder &headers() const noexcept;

private:
  base::result<event> process() noexcept;

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

} // namespace request
} // namespace shared
} // namespace endpoint
} // namespace http3
} // namespace bnl
