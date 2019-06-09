#include <h3c/endpoint/stream/body.hpp>

#include <util/error.hpp>
#include <util/stream.hpp>

namespace h3c {
namespace stream {

body::encoder::encoder(uint64_t id, logger *logger) noexcept
    : id_(id), logger_(logger), frame_(logger)
{}

void body::encoder::add(buffer body, std::error_code &ec)
{
  if (fin_) {
    THROW_VOID(error::internal_error);
  }

  buffers_.emplace(std::move(body));
}

void body::encoder::fin(std::error_code &ec) noexcept
{
  if (state_ == state::fin) {
    THROW_VOID(error::internal_error);
  }

  fin_ = true;

  if (buffers_.empty()) {
    state_ = state::fin;
  }
}

body::encoder::operator state() const noexcept
{
  return state_;
}

quic::data body::encoder::encode(std::error_code &ec) noexcept
{
  // TODO: Implement PRIORITY

  switch (state_) {

    case state::frame: {
      if (buffers_.empty()) {
        STREAM_ENCODE_THROW(error::idle);
      }

      frame frame = frame::payload::data{ buffers_.front().size() };
      buffer encoded = STREAM_ENCODE_TRY(frame_.encode(frame, ec));

      state_ = state::data;

      return { id_, std::move(encoded), false };
    }

    case state::data: {
      buffer body = std::move(buffers_.front());
      buffers_.pop();

      state_ = fin_ && buffers_.empty() ? state::fin : state::frame;

      return { id_, std::move(body), state_ == state::fin };
    }

    case state::fin:
    case state::error:
      NOTREACHED();
  }

  NOTREACHED();
}

body::decoder::decoder(uint64_t id, logger *logger) noexcept
    : id_(id), logger_(logger), frame_(logger)
{}

body::decoder::operator state() const noexcept
{
  return state_;
}

event body::decoder::decode(quic::data &data, std::error_code &ec) noexcept
{
  STREAM_DECODE_START();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

  switch (state_) {

    case state::frame: {
      frame::type type = STREAM_DECODE_TRY(frame_.peek(data.buffer, ec));

      if (type != frame::type::data) {
        STREAM_DECODE_THROW(error::unknown);
      }

      frame frame = STREAM_DECODE_TRY(frame_.decode(data.buffer, ec));

      state_ = state::data;
      remaining_ = frame.data.size;
    }

    case state::data: {
      if (data.buffer.empty()) {
        STREAM_DECODE_THROW(error::incomplete);
      }

      size_t body_part_size = data.buffer.size() < remaining_
                                  ? data.buffer.size()
                                  : static_cast<size_t>(remaining_);
      buffer body_part = data.buffer.slice(body_part_size);

      data.buffer.advance(body_part_size);
      remaining_ -= body_part_size;

      ASSERT(data.buffer.empty() || remaining_ == 0);

      // We've processed all stream data but there still frame data left to be
      // received.
      if (data.fin && remaining_ != 0 && data.buffer.empty()) {
        state_ = state::error;
        STREAM_DECODE_THROW(error::malformed_frame);
      }

      // The stream has ended and we've received the remaining data of the last
      // DATA frame.
      if (data.fin && remaining_ == 0 && data.buffer.empty()) {
        state_ = state::fin;

        // The current frame is done but there's still data left on the stream.
        // We try to read another frame.
      } else if (remaining_ == 0 && !data.buffer.empty()) {
        state_ = state::frame;
      }

      // If the current frame is not done and we haven't received stream fin, we
      // stay in the `data` state to read more frame bytes when we receive more
      // stream data.

      return { id_, state_ == state::fin, std::move(body_part) };
    }

    case state::fin:
    case state::error:
      NOTREACHED();
  }

#pragma GCC diagnostic pop

  NOTREACHED();
}

} // namespace stream
} // namespace h3c
