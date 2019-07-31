#pragma once

#include <bnl/base/buffers.hpp>
#include <bnl/http3/codec/frame.hpp>
#include <bnl/http3/event.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/result.hpp>
#include <bnl/quic/event.hpp>

namespace bnl {
namespace http3 {
namespace endpoint {
namespace stream {
namespace control {

static constexpr uint64_t type = 0x0;

class BNL_HTTP3_EXPORT sender {
public:
  explicit sender(uint64_t id) noexcept;

  sender(sender &&) = default;
  sender &operator=(sender &&) = default;

  result<quic::event> send() noexcept;

private:
  enum class state : uint8_t { type, settings, idle };

  state state_ = state::type;
  settings settings_;

  uint64_t id_;
};

class BNL_HTTP3_EXPORT receiver {
public:
  explicit receiver(uint64_t id) noexcept;

  receiver(receiver &&) = default;
  receiver &operator=(receiver &&) = default;

  virtual ~receiver() noexcept;

  uint64_t id() const noexcept;

  result<void> recv(quic::data data);

  result<event> process() noexcept;

protected:
  virtual result<event> process(frame frame) noexcept = 0;

private:
  enum class state : uint8_t { type, settings, active };

  state state_ = state::type;
  base::buffers buffers_;

  uint64_t id_;
};

}
}
}
}
}
