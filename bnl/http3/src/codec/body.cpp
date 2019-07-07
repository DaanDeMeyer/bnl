#include <bnl/http3/codec/body.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

#include <bnl/base/error.hpp>

namespace bnl {
namespace http3 {
namespace body {

encoder::encoder(const log::api *logger) noexcept
    : logger_(logger), frame_(logger)
{}

base::nothing encoder::add(base::buffer body, std::error_code &ec)
{
  CHECK(!fin_, error::internal_error);

  buffers_.emplace(std::move(body));

  return {};
}

base::nothing encoder::fin(std::error_code &ec) noexcept
{
  CHECK(state_ != state::fin, error::internal_error);

  fin_ = true;

  if (buffers_.empty()) {
    state_ = state::fin;
  }

  return {};
}

bool encoder::finished() const noexcept
{
  return state_ == state::fin;
}

base::buffer encoder::encode(std::error_code &ec) noexcept
{
  // TODO: Implement PRIORITY

  base::state_error_handler<encoder::state> on_error(state_, ec);

  switch (state_) {

    case state::frame: {
      CHECK(!buffers_.empty(), base::error::idle);

      frame frame = frame::payload::data{ buffers_.front().size() };
      base::buffer encoded = TRY(frame_.encode(frame, ec));

      state_ = state::data;

      return encoded;
    }

    case state::data: {
      base::buffer body = std::move(buffers_.front());
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

decoder::decoder(const log::api *logger) noexcept
    : logger_(logger), frame_(logger)
{}

bool decoder::in_progress() const noexcept
{
  return state_ == decoder::state::data;
}

base::buffer decoder::decode(base::buffers &encoded,
                             std::error_code &ec) noexcept
{
  base::state_error_handler<decoder::state> on_error(state_, ec);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

  switch (state_) {

    case state::frame: {
      frame::type type = TRY(frame_.peek(encoded, ec));

      CHECK(type == frame::type::data, base::error::unknown);

      frame frame = TRY(frame_.decode(encoded, ec));

      state_ = state::data;
      remaining_ = frame.data.size;
    }

    case state::data: {
      CHECK(!encoded.empty(), base::error::incomplete);

      size_t body_part_size = encoded.size() < remaining_
                                  ? encoded.size()
                                  : static_cast<size_t>(remaining_);
      base::buffer body_part = encoded.slice(body_part_size);

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

} // namespace body
} // namespace http3
} // namespace bnl
