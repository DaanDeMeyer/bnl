#include <bnl/http3/endpoint/shared/request.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace endpoint {
namespace shared {

request::sender::sender(uint64_t id, const log::api *logger) noexcept
    : id_(id), logger_(logger), headers_(logger), body_(logger)
{}

request::sender::sender(sender &&other) noexcept
    : id_(other.id_),
      logger_(other.logger_),
      headers_(std::move(other.headers_)),
      body_(std::move(other.body_)),
      state_(other.state_),
      handle_(other.handle_)
{
  other.handle_ = nullptr;

  if (handle_ != nullptr) {
    handle_->ref_ = this;
  }
}

request::sender &request::sender::operator=(sender &&other) noexcept
{
  if (&other != this) {
    id_ = other.id_;
    logger_ = other.logger_;
    headers_ = std::move(other.headers_);
    body_ = std::move(other.body_);
    state_ = other.state_;

    handle_ = other.handle_;
    other.handle_ = nullptr;

    if (handle_ != nullptr) {
      handle_->ref_ = this;
    }
  }

  return *this;
}

request::sender::~sender() noexcept
{
  if (handle_ == nullptr) {
    return;
  }

  handle_->ref_ = nullptr;
}

endpoint::handle request::sender::handle() noexcept
{
  endpoint::handle result(id_, this, logger_);

  if (handle_ != nullptr) {
    // Invalidate previous handle.
    handle_->ref_ = nullptr;
  }

  handle_ = &result;

  return result;
}

bool request::sender::finished() const noexcept
{
  return state_ == state::fin;
}

quic::event request::sender::send(std::error_code &ec) noexcept
{
  state_error_handler<sender::state> on_error(state_, ec);

  switch (state_) {

    case state::headers: {
      buffer encoded = TRY(headers_.encode(ec));

      if (headers_.finished()) {
        state_ = state::body;
      }

      return { id_, false, std::move(encoded) };
    }

    case state::body: {
      buffer encoded = TRY(body_.encode(ec));

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

request::receiver::receiver(uint64_t id, const log::api *logger) noexcept
    : id_(id), logger_(logger), frame_(logger), headers_(logger), body_(logger)
{}

request::receiver::~receiver() noexcept = default;

bool request::receiver::closed() const noexcept
{
  return state_ == state::closed;
}

bool request::receiver::finished() const noexcept
{
  return state_ == state::fin;
}

nothing request::receiver::start(std::error_code &ec) noexcept
{
  CHECK(state_ == state::closed, error::internal_error);

  state_ = state::headers;

  return {};
}

const headers::decoder &request::receiver::headers() const noexcept
{
  return headers_;
}

nothing request::receiver::recv(quic::event event,
                                event::handler handler,
                                std::error_code &ec)
{
  state_error_handler<receiver::state> on_error(state_, ec);

  // TODO: Handle `quic::event::type::error`.

  CHECK(!fin_received_, error::internal_error);

  CHECK(event == quic::event::type::data, error::not_implemented);

  buffers_.push(std::move(event.data));
  fin_received_ = event.fin;

  while (!finished()) {
    http3::event result = process(ec);

    if (ec) {
      if (ec == error::incomplete && fin_received_) {
        ec = error::malformed_frame;
      } else if (ec == error::incomplete) {
        ec = {};
      }

      break;
    }

    handler(std::move(result), ec);
  }

  return {};
}

event request::receiver::process(std::error_code &ec) noexcept
{
  buffers::discarder discarder(buffers_);

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
      buffer body = body_.decode(buffers_, ec);
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

  if (ec == error::unknown) {
    frame frame = TRY(frame_.decode(buffers_, ec));

    switch (frame) {
      case frame::type::headers:
        // TODO: Implement trailing HEADERS
        CHECK(state_ != request::receiver::state::body, error::not_implemented);
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

} // namespace shared
} // namespace endpoint
} // namespace http3
} // namespace bnl
