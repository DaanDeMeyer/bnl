#include <h3c/endpoint/stream/control.hpp>

#include <util/error.hpp>
#include <util/stream.hpp>

namespace h3c {
namespace stream {

control::encoder::encoder(uint64_t id, logger *logger) noexcept
    : id_(id), logger_(logger), frame_(logger)
{}

control::encoder::operator state() const noexcept
{
  return state_;
}

quic::data control::encoder::encode(std::error_code &ec) noexcept
{
  switch (state_) {

    case state::settings: {
      buffer encoded = STREAM_ENCODE_TRY(frame_.encode(settings_, ec));
      state_ = state::idle;
      return { id_, std::move(encoded), false };
    }

    case state::idle:
      STREAM_ENCODE_THROW(error::idle);

    case state::error:
      NOTREACHED();
  }

  NOTREACHED();
}

control::decoder::decoder(uint64_t id, logger *logger) noexcept
    : id_(id), logger_(logger), frame_(logger)
{}

control::decoder::operator state() const noexcept
{
  return state_;
}

event control::decoder::decode(quic::data &data, std::error_code &ec) noexcept
{
  STREAM_DECODE_START();

  if (data.fin) {
    STREAM_DECODE_THROW(error::closed_critical_stream);
  }

  switch (state_) {

    case state::settings: {
      frame frame = STREAM_DECODE_TRY(frame_.decode(data.buffer, ec));

      // First frame on the control stream has to be a SETTINGS frame.
      // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-settings
      if (frame != frame::type::settings) {
        STREAM_DECODE_THROW(error::missing_settings);
      }

      state_ = state::active;

      return { id_, false, frame.settings };
    }

    case state::active: {
      frame::type type = STREAM_DECODE_TRY(frame_.peek(data.buffer, ec));

      switch (type) {
        case frame::type::headers: // TODO: STANDARDIZE
        case frame::type::data:
        case frame::type::push_promise:
        case frame::type::duplicate_push:
          STREAM_DECODE_THROW(error::wrong_stream);
        case frame::type::settings:
          STREAM_DECODE_THROW(error::unexpected_frame);
        default:
          break;
      }

      STREAM_DECODE_THROW(error::unknown);
    }

    case state::error:
      NOTREACHED();
  }

  NOTREACHED();
}

} // namespace stream
} // namespace h3c
