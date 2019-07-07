#include <bnl/http3/codec/headers.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

#include <bnl/error.hpp>

namespace bnl {
namespace http3 {
namespace headers {

encoder::encoder(const log::api *logger) noexcept
    : logger_(logger), frame_(logger), qpack_(logger)
{}

nothing encoder::add(header_view header, std::error_code &ec)
{
  CHECK(state_ == state::idle, error::internal_error);

  buffer encoded = qpack_.encode(header, ec);
  buffers_.emplace(std::move(encoded));

  return {};
}

nothing encoder::fin(std::error_code &ec) noexcept
{
  CHECK(state_ == state::idle, error::internal_error);

  state_ = state::frame;

  return {};
}

bool encoder::finished() const noexcept
{
  return state_ == state::fin;
}

buffer encoder::encode(std::error_code &ec) noexcept
{
  state_error_handler<encoder::state> on_error(state_, ec);

  switch (state_) {

    case state::idle:
      THROW(core::error::idle);

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

decoder::decoder(const log::api *logger) noexcept
    : logger_(logger), frame_(logger), qpack_(logger)
{}

bool decoder::started() const noexcept
{
  return state_ != state::frame;
}

bool decoder::finished() const noexcept
{
  return state_ == state::fin;
}

header decoder::decode(buffers &encoded, std::error_code &ec) noexcept
{
  state_error_handler<decoder::state> on_error(state_, ec);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

  switch (state_) {

    case state::frame: {
      frame::type type = TRY(frame_.peek(encoded, ec));

      CHECK(type == frame::type::headers, core::error::unknown);

      frame frame = TRY(frame_.decode(encoded, ec));

      state_ = state::qpack;
      headers_size_ = frame.headers.size;

      CHECK(headers_size_ != 0, error::malformed_frame);
    }

    case state::qpack: {
      header header = TRY(qpack_.decode(encoded, ec));

      CHECK(qpack_.count() <= headers_size_, error::malformed_frame);

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

} // namespace headers
} // namespace http3
} // namespace bnl
