#include <h3c/endpoint/stream/headers.hpp>

#include <util/error.hpp>

namespace h3c {
namespace stream {

headers::encoder::encoder(const logger *logger) noexcept
    : logger_(logger), frame_(logger), qpack_(logger)
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

bool headers::encoder::finished() const noexcept
{
  return state_ == state::fin;
}

buffer headers::encoder::encode(std::error_code &ec) noexcept
{
  state_error_handler<encoder::state> on_error(state_, ec);

  switch (state_) {

    case state::idle:
      THROW(error::idle);

    case state::frame: {
      frame frame = frame::payload::headers{ qpack_.count() };

      buffer encoded = TRY(frame_.encode(frame, ec));

      state_ = state::qpack;

      return encoded;
    }

    case state::qpack: {
      buffer encoded = std::move(buffers_.front());
      buffers_.pop();

      state_ = buffers_.empty() ? state::fin : state_;

      return encoded;
    }

    case state::fin:
    case state::error:
      THROW(error::internal_error);
  }

  NOTREACHED();
}

headers::decoder::decoder(const logger *logger) noexcept
    : logger_(logger), frame_(logger), qpack_(logger)
{}

bool headers::decoder::started() const noexcept
{
  return state_ != state::frame;
}

bool headers::decoder::finished() const noexcept
{
  return state_ == state::fin;
}

header headers::decoder::decode(buffers &encoded, std::error_code &ec) noexcept
{
  state_error_handler<decoder::state> on_error(state_, ec);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

  switch (state_) {

    case state::frame: {
      frame::type type = TRY(frame_.peek(encoded, ec));

      if (type != frame::type::headers) {
        THROW(error::unknown);
      }

      frame frame = TRY(frame_.decode(encoded, ec));

      state_ = state::qpack;
      headers_size_ = frame.headers.size;

      if (headers_size_ == 0) {
        THROW(error::malformed_frame);
      }
    }

    case state::qpack: {
      header header = TRY(qpack_.decode(encoded, ec));

      if (qpack_.count() > headers_size_) {
        THROW(error::malformed_frame);
      }

      bool fin = qpack_.count() == headers_size_;
      state_ = fin ? state::fin : state_;

      return header;
    }

    case state::fin:
    case state::error:
      THROW(error::internal_error);
  }

#pragma GCC diagnostic pop

  NOTREACHED();
}

} // namespace stream
} // namespace h3c
