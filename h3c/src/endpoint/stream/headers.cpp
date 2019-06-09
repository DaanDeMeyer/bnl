#include <h3c/endpoint/stream/headers.hpp>

#include <util/error.hpp>
#include <util/stream.hpp>

namespace h3c {
namespace stream {

headers::encoder::encoder(uint64_t id, logger *logger) noexcept
    : id_(id), logger_(logger), frame_(logger), qpack_(logger)
{}

void headers::encoder::add(header_view header, std::error_code &ec)
{
  if (state_ != state::idle) {
    THROW_VOID(error::internal_error);
  }

  buffer encoded = qpack_.encode(header, ec);
  buffers_.emplace(std::move(encoded));
}

void headers::encoder::fin(std::error_code &ec) noexcept
{
  if (state_ != state::idle) {
    THROW_VOID(error::internal_error);
  }

  state_ = state::frame;
}

headers::encoder::operator state() const noexcept
{
  return state_;
}

quic::data headers::encoder::encode(std::error_code &ec) noexcept
{
  switch (state_) {

    case state::idle:
      STREAM_ENCODE_THROW(error::idle);

    case state::frame: {
      frame frame = frame::payload::headers{ qpack_.count() };

      buffer encoded = STREAM_ENCODE_TRY(frame_.encode(frame, ec));

      state_ = state::qpack;

      return { id_, std::move(encoded), false };
    }

    case state::qpack: {
      buffer encoded = std::move(buffers_.front());
      buffers_.pop();

      state_ = buffers_.empty() ? state::fin : state_;

      return { id_, std::move(encoded), false };
    }

    case state::fin:
    case state::error:
      NOTREACHED();
  }

  NOTREACHED();
}

headers::decoder::decoder(uint64_t id, logger *logger) noexcept
    : id_(id), logger_(logger), frame_(logger), qpack_(logger)
{}

headers::decoder::operator state() const noexcept
{
  return state_;
}

event headers::decoder::decode(quic::data &data, std::error_code &ec) noexcept
{
  STREAM_DECODE_START();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

  switch (state_) {

    case state::frame: {
      frame::type type = STREAM_DECODE_TRY(frame_.peek(data.buffer, ec));

      if (type != frame::type::headers) {
        STREAM_DECODE_THROW(error::unknown);
      }

      frame frame = STREAM_DECODE_TRY(frame_.decode(data.buffer, ec));

      state_ = state::qpack;
      headers_size_ = frame.headers.size;

      if (headers_size_ == 0) {
        STREAM_DECODE_THROW(error::malformed_frame);
      }
    }

    case state::qpack: {
      header header = STREAM_DECODE_TRY(qpack_.decode(data.buffer, ec));

      if (qpack_.count() > headers_size_) {
        STREAM_DECODE_THROW(error::malformed_frame);
      }

      bool fin = qpack_.count() == headers_size_;
      state_ = fin ? state::fin : state_;

      return { id_, fin, std::move(header) };
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
