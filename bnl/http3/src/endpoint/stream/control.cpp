#include <bnl/http3/endpoint/stream/control.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/log.hpp>

namespace bnl {
namespace http3 {
namespace endpoint {
namespace stream {
namespace control {

sender::sender(uint64_t id) noexcept
  : id_(id)
{}

result<quic::event>
sender::send() noexcept
{
  switch (state_) {
    case state::type: {
      base::buffer encoded = BNL_TRY(varint::encode(type));
      state_ = state::settings;
      return quic::data{ id_, false, std::move(encoded) };
    }

    case state::settings: {
      base::buffer encoded = BNL_TRY(frame::encode(settings_));
      state_ = state::idle;
      return quic::data{ id_, false, std::move(encoded) };
    }

    case state::idle:
      return base::error::idle;
  }

  assert(false);
  return connection::error::internal;
}

receiver::receiver(uint64_t id) noexcept
  : id_(id)
{}

receiver::~receiver() noexcept = default;

uint64_t
receiver::id() const noexcept
{
  return id_;
}

result<void>
receiver::recv(quic::data data)
{
  if (data.fin) {
    return connection::error::closed_critical_stream;
  }

  buffers_.push(std::move(data.buffer));

  return success();
}

result<event>
receiver::process() noexcept
{
  switch (state_) {

    // TODO: Move this out of control stream since we technically won't know its
    // the control stream until we've decoded the type.
    case state::type: {
      uint64_t type = BNL_TRY(varint::decode(buffers_));
      (void) type; // Silence GCC 9 Wunused-variable warning
      assert(type == control::type);
      state_ = state::settings;
    }
    /* FALLTHRU */
    case state::settings: {
      frame frame = BNL_TRY(frame::decode(buffers_));

      // First frame on the control stream has to be a SETTINGS frame.
      // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-settings
      if (frame != frame::type::settings) {
        BNL_LOG_E(
          "First frame on the control stream ({}) is not a SETTINGS frame",
          frame);
        return connection::error::missing_settings;
      };

      state_ = state::active;

      return event::payload::settings{ frame.settings };
    }

    case state::active: {
      frame frame = BNL_TRY(frame::decode(buffers_));

      switch (frame) {
        case frame::type::headers: // TODO: STANDARDIZE
        case frame::type::data:
        case frame::type::push_promise:
        case frame::type::duplicate_push:
          return connection::error::wrong_stream;
        case frame::type::settings:
          return connection::error::unexpected_frame;
        default:
          break;
      }

      return process(frame);
    }
  }

  assert(false);
  return connection::error::internal;
}

}
}
}
}
}
