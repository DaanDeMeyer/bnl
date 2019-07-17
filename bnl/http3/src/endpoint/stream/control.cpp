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
  : frame_(logger)
  , id_(id)
  , logger_(logger)
{}

base::result<quic::event>
sender::send() noexcept
{
  switch (state_) {
    case state::settings: {
      base::buffer encoded = TRY(frame_.encode(settings_));
      state_ = state::idle;
      return quic::event(id_, false, std::move(encoded));
    }

    case state::idle:
      THROW(base::error::idle);
  }

  NOTREACHED();
}

receiver::receiver(uint64_t id, const log::api *logger) noexcept
  : frame_(logger)
  , id_(id)
  , logger_(logger)
{}

receiver::~receiver() noexcept = default;

uint64_t
receiver::id() const noexcept
{
  return id_;
}

std::error_code
receiver::recv(quic::event event, event::handler handler)
{
  CHECK(event == quic::event::type::data, base::error::not_implemented);
  CHECK(!event.fin, error::closed_critical_stream);

  buffers_.push(std::move(event.data));

  while (true) {
    base::result<http3::event> result = process();
    if (!result) {
      if (result == base::error::incomplete) {
        return {};
      }

      return result.error();
    }

    TRY(handler(std::move(result.value())));
  }

  return {};
}

base::result<event>
receiver::process() noexcept
{
  switch (state_) {

    case state::settings: {
      frame frame = TRY(frame_.decode(buffers_));

      // First frame on the control stream has to be a SETTINGS frame.
      // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-settings
      CHECK(frame == frame::type::settings, error::missing_settings);

      state_ = state::active;

      return event{ id_, false, frame.settings };
    }

    case state::active: {
      frame frame = TRY(frame_.decode(buffers_));

      switch (frame) {
        case frame::type::headers: // TODO: STANDARDIZE
        case frame::type::data:
        case frame::type::push_promise:
        case frame::type::duplicate_push:
          THROW(error::wrong_stream);
        case frame::type::settings:
          THROW(error::unexpected_frame);
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
