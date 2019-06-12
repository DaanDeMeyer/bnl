#include <h3c/endpoint/stream/control.hpp>

#include <util/error.hpp>

namespace h3c {
namespace stream {

control::sender::sender(uint64_t id, const logger *logger) noexcept
    : id_(id), logger_(logger), frame_(logger)
{}

quic::data control::sender::send(std::error_code &ec) noexcept
{
  state_error_handler<sender::state> on_error(state_, ec);

  switch (state_) {

    case state::settings: {
      buffer encoded = TRY(frame_.encode(settings_, ec));
      state_ = state::idle;
      return { id_, std::move(encoded), false };
    }

    case state::idle:
      THROW(error::idle);

    case state::error:
      THROW(error::internal_error);
  }

  NOTREACHED();
}

control::receiver::receiver(uint64_t id, const logger *logger) noexcept
    : id_(id), logger_(logger), frame_(logger)
{}

control::receiver::~receiver() noexcept = default;

void control::receiver::recv(quic::data data,
                             event::handler handler,
                             std::error_code &ec)
{
  state_error_handler<receiver::state> on_error(state_, ec);

  if (data.fin) {
    THROW_VOID(error::closed_critical_stream);
  }

  buffers_.push(std::move(data.buffer));

  while (true) {
    event event = process(ec);
    if (ec) {
      if (ec == error::incomplete) {
        ec = {};
      }

      break;
    }

    handler(std::move(event), ec);
  }
}

event control::receiver::process(std::error_code &ec) noexcept
{
  buffers::discarder discarder(buffers_);

  switch (state_) {

    case state::settings: {
      frame frame = TRY(frame_.decode(buffers_, ec));

      // First frame on the control stream has to be a SETTINGS frame.
      // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-settings
      if (frame != frame::type::settings) {
        THROW(error::missing_settings);
      }

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

} // namespace stream
} // namespace h3c
