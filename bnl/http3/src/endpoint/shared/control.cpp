#include <bnl/http3/endpoint/shared/control.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace endpoint {
namespace shared {

control::sender::sender(uint64_t id, const log::api *logger) noexcept
    : id_(id), logger_(logger), frame_(logger)
{}

quic::event control::sender::send(std::error_code &ec) noexcept
{
  state_error_handler<sender::state> on_error(state_, ec);

  switch (state_) {
    case state::settings: {
      buffer encoded = TRY(frame_.encode(settings_, ec));
      state_ = state::idle;
      return { id_, false, std::move(encoded) };
    }

    case state::idle:
      THROW(error::idle);

    case state::error:
      THROW(error::internal_error);
  }

  NOTREACHED();
}

control::receiver::receiver(uint64_t id, const log::api *logger) noexcept
    : id_(id), logger_(logger), frame_(logger)
{}

control::receiver::~receiver() noexcept = default;

uint64_t control::receiver::id() const noexcept
{
  return id_;
}

nothing control::receiver::recv(quic::event event,
                                event::handler handler,
                                std::error_code &ec)
{
  state_error_handler<receiver::state> on_error(state_, ec);

  CHECK(event == quic::event::type::data, error::not_implemented);

  CHECK(!event.fin, error::closed_critical_stream);

  buffers_.push(std::move(event.data));

  while (true) {
    http3::event result = process(ec);
    if (ec) {
      if (ec == error::incomplete) {
        ec = {};
      }

      break;
    }

    handler(std::move(result), ec);
  }

  return {};
}

event control::receiver::process(std::error_code &ec) noexcept
{
  buffers::discarder discarder(buffers_);

  switch (state_) {

    case state::settings: {
      frame frame = TRY(frame_.decode(buffers_, ec));

      // First frame on the control stream has to be a SETTINGS frame.
      // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-settings
      CHECK(frame == frame::type::settings, error::missing_settings);

      state_ = state::active;

      return { id_, false, frame.settings };
    }

    case state::active: {
      frame frame = TRY(frame_.decode(buffers_, ec));

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

      return process(frame, ec);
    }

    case state::error:
      THROW(error::internal_error);
  }

  NOTREACHED();
}

} // namespace shared
} // namespace endpoint
} // namespace http3
} // namespace bnl
