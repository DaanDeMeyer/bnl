#pragma once

#include <bnl/base/buffers.hpp>
#include <bnl/base/macro.hpp>
#include <bnl/http3/codec/frame.hpp>
#include <bnl/http3/event.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/quic/event.hpp>

namespace bnl {

namespace log {
class api;
} // namespace log

namespace http3 {
namespace endpoint {
namespace shared {
namespace control {

class BNL_HTTP3_EXPORT sender {
public:
  sender(uint64_t id, const log::api *logger) noexcept;

  BNL_BASE_MOVE_ONLY(sender);

  base::result<quic::event> send() noexcept;

private:
  enum class state : uint8_t { settings, idle };

  state state_ = state::settings;
  settings settings_;

  frame::encoder frame_;

  uint64_t id_;
  const log::api *logger_;
};

class BNL_HTTP3_EXPORT receiver {
public:
  receiver(uint64_t id, const log::api *logger) noexcept;

  BNL_BASE_NO_COPY(receiver);
  BNL_BASE_DEFAULT_MOVE(receiver);

  virtual ~receiver() noexcept;

  uint64_t id() const noexcept;

  std::error_code recv(quic::event event, event::handler handler);

protected:
  virtual base::result<event> process(frame frame) noexcept = 0;

private:
  base::result<event> process() noexcept;

private:
  enum class state : uint8_t { settings, active };

  state state_ = state::settings;
  base::buffers buffers_;

  frame::decoder frame_;

  uint64_t id_;
  const log::api *logger_;
};

} // namespace control
} // namespace shared
} // namespace endpoint
} // namespace http3
} // namespace bnl
