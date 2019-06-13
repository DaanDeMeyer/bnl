#pragma once

#include <bnl/http3/event.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/quic.hpp>

#include <bnl/http3/codec/frame.hpp>

#include <bnl/buffers.hpp>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace stream {
namespace control {

class BNL_HTTP3_EXPORT sender {
public:
  sender(uint64_t id, const log::api *logger) noexcept;

  quic::data send(std::error_code &ec) noexcept;

private:
  uint64_t id_;
  const log::api *logger_;

  frame::encoder frame_;

  enum class state : uint8_t { settings, idle, error };

  state state_ = state::settings;
  settings settings_;
};

class BNL_HTTP3_EXPORT receiver {
public:
  receiver(uint64_t id, const log::api *logger) noexcept;

  receiver(const receiver &) = default;
  receiver &operator=(const receiver &) = default;

  receiver(receiver &&) = default;
  receiver &operator=(receiver &&) = default;

  virtual ~receiver() noexcept;

  void recv(quic::data data, event::handler handler, std::error_code &ec);

protected:
  virtual event process(frame frame, std::error_code &ec) = 0;

private:
  event process(std::error_code &ec) noexcept;

  uint64_t id_;
  const log::api *logger_;

  frame::decoder frame_;

  enum class state : uint8_t { settings, active, error };

  state state_ = state::settings;

  buffers buffers_;
};

} // namespace control
} // namespace stream
} // namespace http3
} // namespace bnl
