#include <h3c/endpoint/stream/body.hpp>

#include <util/error.hpp>

namespace h3c {
namespace stream {

body::encoder::encoder(logger *logger) noexcept
    : logger_(logger), frame_(logger)
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

bool body::encoder::finished() const noexcept
{
  return state_ == state::fin;
}

buffer body::encoder::encode(std::error_code &ec) noexcept
{
  // TODO: Implement PRIORITY

  state_error_handler<encoder::state> on_error(state_, ec);

  switch (state_) {

    case state::frame: {
      if (buffers_.empty()) {
        THROW(error::idle);
      }

      frame frame = frame::payload::data{ buffers_.front().size() };
      buffer encoded = TRY(frame_.encode(frame, ec));

      state_ = state::data;

      return encoded;
    }

    case state::data: {
      buffer body = std::move(buffers_.front());
      buffers_.pop();

      state_ = fin_ && buffers_.empty() ? state::fin : state::frame;

      return body;
    }

    case state::fin:
    case state::error:
      THROW(error::internal_error);
  }

  NOTREACHED();
}

body::decoder::decoder(logger *logger) noexcept
    : logger_(logger), frame_(logger)
{}

bool body::decoder::in_progress() const noexcept
{
  return state_ == body::decoder::state::data;
}

buffer body::decoder::decode(buffers &encoded, std::error_code &ec) noexcept
{
  state_error_handler<decoder::state> on_error(state_, ec);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

  switch (state_) {

    case state::frame: {
      frame::type type = TRY(frame_.peek(encoded, ec));

      if (type != frame::type::data) {
        THROW(error::unknown);
      }

      frame frame = TRY(frame_.decode(encoded, ec));

      state_ = state::data;
      remaining_ = frame.data.size;
    }

    case state::data: {
      if (encoded.empty()) {
        THROW(error::incomplete);
      }

      size_t body_part_size = encoded.size() < remaining_
                                  ? encoded.size()
                                  : static_cast<size_t>(remaining_);
      buffer body_part = encoded.slice(body_part_size);

      encoded += body_part_size;
      remaining_ -= body_part_size;

      ASSERT(encoded.empty() || remaining_ == 0);

      state_ = remaining_ == 0 ? state::frame : state_;

      return body_part;
    }

    case state::error:
      THROW(error::internal_error);
  }

#pragma GCC diagnostic pop

  NOTREACHED();
}

} // namespace stream
} // namespace h3c
