#include <bnl/http3/endpoint/shared/request.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace endpoint {
namespace shared {
namespace request {

sender::sender(uint64_t id, const log::api *logger) noexcept
    : id_(id), logger_(logger), headers_(logger), body_(logger)
{}

bool sender::finished() const noexcept
{
  return state_ == state::fin;
}

quic::event sender::send(std::error_code &ec) noexcept
{
  base::state_error_handler<sender::state> on_error(state_, ec);

  switch (state_) {

    case state::headers: {
      base::buffer encoded = TRY(headers_.encode(ec));

      if (headers_.finished()) {
        state_ = state::body;
      }

      return { id_, false, std::move(encoded) };
    }

    case state::body: {
      base::buffer encoded = TRY(body_.encode(ec));

      if (body_.finished()) {
        state_ = state::fin;
      }

      return { id_, body_.finished(), std::move(encoded) };
    }

    case state::fin:
    case state::error:
      THROW(error::internal_error);
  }

  NOTREACHED();
}

base::nothing sender::header(header_view header, std::error_code &ec)
{
  return headers_.add(header, ec);
}

base::nothing sender::body(base::buffer body, std::error_code &ec)
{
  return body_.add(std::move(body), ec);
}

base::nothing sender::start(std::error_code &ec) noexcept
{
  return headers_.fin(ec);
}

base::nothing sender::fin(std::error_code &ec) noexcept
{
  return body_.fin(ec);
}

receiver::receiver(uint64_t id, const log::api *logger) noexcept
    : id_(id), logger_(logger), frame_(logger), headers_(logger), body_(logger)
{}

receiver::~receiver() noexcept = default;

bool receiver::closed() const noexcept
{
  return state_ == state::closed;
}

bool receiver::finished() const noexcept
{
  return state_ == state::fin;
}

base::nothing receiver::start(std::error_code &ec) noexcept
{
  CHECK(state_ == state::closed, error::internal_error);

  state_ = state::headers;

  return {};
}

const headers::decoder &receiver::headers() const noexcept
{
  return headers_;
}

base::nothing receiver::recv(quic::event event,
                             event::handler handler,
                             std::error_code &ec)
{
  base::state_error_handler<receiver::state> on_error(state_, ec);

  // TODO: Handle `quic::event::type::error`.

  CHECK(!fin_received_, error::internal_error);

  CHECK(event == quic::event::type::data, base::error::not_implemented);

  buffers_.push(std::move(event.data));
  fin_received_ = event.fin;

  while (!finished()) {
    http3::event result = process(ec);

    if (ec) {
      if (ec == base::error::incomplete && fin_received_) {
        ec = error::malformed_frame;
      } else if (ec == base::error::incomplete) {
        ec = {};
      }

      break;
    }

    handler(std::move(result), ec);
  }

  return {};
}

event receiver::process(std::error_code &ec) noexcept
{
  switch (state_) {

    case state::closed:
      THROW(error::internal_error);

    case state::headers: {
      header header = headers_.decode(buffers_, ec);
      if (ec) {
        break;
      }

      bool fin = buffers_.empty() && fin_received_;

      if (headers_.finished() && fin) {
        state_ = state::fin;
      } else if (headers_.finished()) {
        state_ = state::body;
      }

      return { id_, headers_.finished(), std::move(header) };
    }

    case state::body: {
      base::buffer body = body_.decode(buffers_, ec);
      if (ec) {
        break;
      }

      bool fin = buffers_.empty() && fin_received_;

      if (fin) {
        // We've processed all stream data but there still frame data left to be
        // received.
        CHECK(!body_.in_progress(), error::malformed_frame);
        state_ = state::fin;
      }

      return { id_, fin, std::move(body) };
    }

    case state::fin:
    case state::error:
      THROW(error::internal_error);
  };

  if (ec == base::error::unknown) {
    frame frame = TRY(frame_.decode(buffers_, ec));

    switch (frame) {
      case frame::type::headers:
        // TODO: Implement trailing HEADERS
        CHECK(state_ != receiver::state::body, base::error::not_implemented);
        break;
      case frame::type::data:
        THROW(error::unexpected_frame);
      case frame::type::settings:
      case frame::type::max_push_id:
      case frame::type::cancel_push:
      case frame::type::goaway:
        THROW(error::wrong_stream);
      default:
        return process(frame, ec);
    }
  }

  return {};
}

} // namespace request
} // namespace shared
} // namespace endpoint
} // namespace http3
} // namespace bnl
