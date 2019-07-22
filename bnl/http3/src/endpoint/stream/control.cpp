#include <bnl/http3/endpoint/stream/control.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace endpoint {
namespace stream {
namespace control {

sender::sender(uint64_t id, const log::api *logger) noexcept
  : varint_(logger)
  , frame_(logger)
  , id_(id)
  , logger_(logger)
{}

result<quic::event>
sender::send() noexcept
{
  switch (state_) {
    case state::type: {
      base::buffer encoded = TRY(varint_.encode(type));
      state_ = state::settings;
      return quic::data{ id_, false, std::move(encoded) };
    }

    case state::settings: {
      base::buffer encoded = TRY(frame_.encode(settings_));
      state_ = state::idle;
      return quic::data{ id_, false, std::move(encoded) };
    }

    case state::idle:
      return base::error::idle;
  }

  NOTREACHED();
}

receiver::receiver(uint64_t id, const log::api *logger) noexcept
  : varint_(logger)
  , frame_(logger)
  , id_(id)
  , logger_(logger)
{}

receiver::~receiver() noexcept = default;

uint64_t
receiver::id() const noexcept
{
  return id_;
}

result<void>
receiver::recv(quic::data data, event::handler handler)
{
  if (data.fin) {
    THROW(connection::error::closed_critical_stream);
  }

  buffers_.push(std::move(data.buffer));

  while (true) {
    result<http3::event> r = process();
    if (!r) {
      if (r.error() == base::error::incomplete) {
        return success();
      }

      return std::move(r).error();
    }

    TRY(handler(std::move(r.value())));
  }

  return success();
}

result<event>
receiver::process() noexcept
{
  switch (state_) {

    // TODO: Move this out of control stream since we technically won't know its
    // the control stream until we've decoded the type.
    case state::type: {
      uint64_t type = TRY(varint_.decode(buffers_));
      assert(type == control::type);
      state_ = state::settings;
    }

    case state::settings: {
      frame frame = TRY(frame_.decode(buffers_));

      // First frame on the control stream has to be a SETTINGS frame.
      // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-settings
      if (frame != frame::type::settings) {
        LOG_E("First frame on the control stream ({}) is not a SETTINGS frame",
              frame);
        THROW(connection::error::missing_settings);
      };

      state_ = state::active;

      return event::payload::settings{ frame.settings };
    }

    case state::active: {
      frame frame = TRY(frame_.decode(buffers_));

      switch (frame) {
        case frame::type::headers: // TODO: STANDARDIZE
        case frame::type::data:
        case frame::type::push_promise:
        case frame::type::duplicate_push:
          THROW(connection::error::wrong_stream);
        case frame::type::settings:
          THROW(connection::error::unexpected_frame);
        default:
          break;
      }

      return process(frame);
    }
  }

  NOTREACHED();
}

}
}
}
}
}
