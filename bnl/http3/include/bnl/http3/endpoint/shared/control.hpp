#pragma once

#include <bnl/base/buffers.hpp>
#include <bnl/base/macro.hpp>
#include <bnl/base/nothing.hpp>
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

  quic::event send(std::error_code &ec) noexcept;

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

  BNL_BASE_NO_COPY(receiver);
  BNL_BASE_DEFAULT_MOVE(receiver);

  virtual ~receiver() noexcept;

  uint64_t id() const noexcept;

  base::nothing recv(quic::event event,
                     event::handler handler,
                     std::error_code &ec);

protected:
  virtual event process(frame frame, std::error_code &ec) = 0;

private:
  event process(std::error_code &ec) noexcept;

  uint64_t id_;
  const log::api *logger_;

  frame::decoder frame_;

  enum class state : uint8_t { settings, active, error };

  state state_ = state::settings;

  base::buffers buffers_;
};

} // namespace control
} // namespace shared
} // namespace endpoint
} // namespace http3
} // namespace bnl
