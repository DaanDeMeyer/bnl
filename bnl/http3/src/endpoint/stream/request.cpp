#include <bnl/http3/endpoint/stream/request.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace endpoint {
namespace stream {
namespace request {

sender::sender(uint64_t id, const log::api *logger) noexcept
  : headers_(logger)
  , body_(logger)
  , id_(id)
  , logger_(logger)
{}

sender::sender(sender &&other) noexcept
  : handle_(other.handle_)
  , state_(other.state_)
  , headers_(std::move(other.headers_))
  , body_(std::move(other.body_))
  , id_(other.id_)
  , logger_(other.logger_)
{
  if (handle_ != nullptr) {
    handle_->sender_ = this;
  }

  other.handle_ = nullptr;
}

sender &
sender::operator=(sender &&other) noexcept
{
  if (&other != this) {
    handle_ = other.handle_;
    state_ = other.state_;
    headers_ = std::move(other.headers_);
    body_ = std::move(other.body_);
    id_ = other.id_;
    logger_ = other.logger_;

    if (handle_ != nullptr) {
      handle_->sender_ = this;
    }

    other.handle_ = nullptr;
  }

  return *this;
}

sender::~sender() noexcept
{
  if (handle_ != nullptr) {
    handle_->sender_ = nullptr;
  }
}

bool
sender::finished() const noexcept
{
  return state_ == state::fin;
}

base::result<quic::event>
sender::send() noexcept
{
  switch (state_) {

    case state::headers: {
      base::buffer encoded = TRY(headers_.encode());

      if (headers_.finished()) {
        state_ = state::body;
      }

      return quic::event(id_, false, std::move(encoded));
    }

    case state::body: {
      base::buffer encoded = TRY(body_.encode());

      if (body_.finished()) {
        state_ = state::fin;
      }

      return quic::event(id_, body_.finished(), std::move(encoded));
    }

    case state::fin:
      THROW(error::internal_error);
  }

  NOTREACHED();
}

std::error_code
sender::header(header_view header)
{
  return headers_.add(header);
}

std::error_code
sender::body(base::buffer body)
{
  return body_.add(std::move(body));
}

std::error_code
sender::start() noexcept
{
  return headers_.fin();
}

std::error_code
sender::fin() noexcept
{
  return body_.fin();
}

sender::handle::handle(sender *sender)
  : id_(sender->id_)
  , sender_(sender)
{
  sender_->handle_ = this;
}

sender::handle::handle(handle &&other) noexcept
  : handle()
{
  operator=(std::move(other));
}

sender::handle &
sender::handle::operator=(sender::handle &&other) noexcept
{
  if (&other != this) {
    id_ = other.id_;
    sender_ = other.sender_;
    other.id_ = UINT64_MAX;
    other.sender_ = nullptr;

    if (sender_ != nullptr) {
      sender_->handle_ = this;
    }
  }

  return *this;
}

sender::handle::~handle()
{
  if (sender_ != nullptr) {
    sender_->handle_ = nullptr;
  }
}

uint64_t sender::handle::id() const noexcept
{
  return id_;
}

std::error_code
sender::handle::header(header_view header)
{
  if (sender_ == nullptr) {
    return error::invalid_handle;
  }

  return sender_->header(header);
}

std::error_code
sender::handle::body(base::buffer body)
{
  if (sender_ == nullptr) {
    return error::invalid_handle;
  }

  return sender_->body(std::move(body));
}

std::error_code
sender::handle::start() noexcept
{
  if (sender_ == nullptr) {
    return error::invalid_handle;
  }

  return sender_->start();
}

std::error_code
sender::handle::fin() noexcept
{
  if (sender_ == nullptr) {
    return error::invalid_handle;
  }

  return sender_->fin();
}

receiver::receiver(uint64_t id, const log::api *logger) noexcept
  : frame_(logger)
  , headers_(logger)
  , body_(logger)
  , id_(id)
  , logger_(logger)
{}

receiver::~receiver() noexcept = default;

bool
receiver::closed() const noexcept
{
  return state_ == state::closed;
}

bool
receiver::finished() const noexcept
{
  return state_ == state::fin;
}

std::error_code
receiver::start() noexcept
{
  CHECK(state_ == state::closed, error::internal_error);

  state_ = state::headers;

  return {};
}

const headers::decoder &
receiver::headers() const noexcept
{
  return headers_;
}

std::error_code
receiver::recv(quic::event event, event::handler handler)
{
  // TODO: Handle `quic::event::type::error`.

  CHECK(!fin_received_, error::internal_error);

  CHECK(event == quic::event::type::data, base::error::not_implemented);

  buffers_.push(std::move(event.data));
  fin_received_ = event.fin;

  while (!finished()) {
    base::result<http3::event> result = process();

    if (!result) {
      if (result == base::error::incomplete && fin_received_) {
        return error::malformed_frame;
      }

      if (result == base::error::incomplete) {
        return {};
      }

      return result.error();
    }

    TRY(handler(std::move(result.value())));
  }

  return {};
}

base::result<event>
receiver::process() noexcept
{
  std::error_code ec;

  switch (state_) {

    case state::closed:
      THROW(error::internal_error);

    case state::headers: {
      base::result<header> result = headers_.decode(buffers_);
      if (!result) {
        ec = result.error();
        break;
      }

      bool fin = buffers_.empty() && fin_received_;

      if (headers_.finished() && fin) {
        state_ = state::fin;
      } else if (headers_.finished()) {
        state_ = state::body;
      }

      return event(id_, headers_.finished(), std::move(result.value()));
    }

    case state::body: {
      base::result<base::buffer> result = body_.decode(buffers_);
      if (!result) {
        ec = result.error();
        break;
      }

      bool fin = buffers_.empty() && fin_received_;

      if (fin) {
        // We've processed all stream data but there still frame data left to be
        // received.
        CHECK(!body_.in_progress(), error::malformed_frame);
        state_ = state::fin;
      }

      return event(id_, fin, std::move(result.value()));
    }

    case state::fin:
      THROW(error::internal_error);
  };

  if (ec == base::error::unknown) {
    frame frame = TRY(frame_.decode(buffers_));

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
        return process(frame);
    }
  }

  return ec;
}

}
}
}
}
}
