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

result<quic::event>
sender::send() noexcept
{
  switch (state_) {

    case state::headers: {
      base::buffer encoded = TRY(headers_.encode());

      if (headers_.finished()) {
        state_ = body_.finished() ? state::fin : state::body;
      }

      return quic::data{ id_, false, std::move(encoded) };
    }

    case state::body: {
      base::buffer encoded = TRY(body_.encode());

      if (body_.finished()) {
        state_ = state::fin;
      }

      return quic::data{ id_, body_.finished(), std::move(encoded) };
    }

    case state::fin:
      THROW(connection::error::internal);
  }

  NOTREACHED();
}

result<void>
sender::header(header_view header)
{
  return headers_.add(header);
}

result<void>
sender::body(base::buffer body)
{
  return body_.add(std::move(body));
}

result<void>
sender::start() noexcept
{
  return headers_.fin();
}

result<void>
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

uint64_t
sender::handle::id() const noexcept
{
  return id_;
}

result<void>
sender::handle::header(header_view header)
{
  if (sender_ == nullptr) {
    return error::invalid_handle;
  }

  return sender_->header(header);
}

result<void>
sender::handle::body(base::buffer body)
{
  if (sender_ == nullptr) {
    return error::invalid_handle;
  }

  return sender_->body(std::move(body));
}

result<void>
sender::handle::start() noexcept
{
  if (sender_ == nullptr) {
    return error::invalid_handle;
  }

  return sender_->start();
}

result<void>
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

result<void>
receiver::start() noexcept
{
  assert(state_ == state::closed);

  state_ = state::headers;

  return bnl::success();
}

const headers::decoder &
receiver::headers() const noexcept
{
  return headers_;
}

result<void>
receiver::recv(quic::data data, event::handler handler)
{
  if (fin_received_) {
    THROW(connection::error::internal);
  }

  fin_received_ = data.fin;
  buffers_.push(std::move(data.buffer));

  while (!finished()) {
    result<http3::event> r = process();

    if (!r) {
      if (r.error() == base::error::incomplete && fin_received_) {
        return connection::error::malformed_frame;
      }

      if (r.error() == base::error::incomplete) {
        return bnl::success();
      }

      return std::move(r).error();
    }

    TRY(handler(std::move(r.value())));
  }

  return bnl::success();
}

result<event>
receiver::process() noexcept
{
  result<void>::error_type sc;

  switch (state_) {

    case state::closed:
      THROW(connection::error::internal);

    case state::headers: {
      result<header> r = headers_.decode(buffers_);
      if (!r) {
        sc = std::move(r).error();
        break;
      }

      bool fin = buffers_.empty() && fin_received_;

      if (headers_.finished() && fin) {
        state_ = state::fin;
      } else if (headers_.finished()) {
        state_ = state::body;
      }

      return event::payload::header{ id_,
                                     headers_.finished(),
                                     std::move(r).value() };
    }

    case state::body: {
      result<base::buffer> r = body_.decode(buffers_);
      if (!r) {
        sc = std::move(r).error();
        break;
      }

      bool fin = buffers_.empty() && fin_received_;

      if (fin) {
        // We've processed all stream data but there still frame data left to be
        // received.
        if (body_.in_progress()) {
          THROW(connection::error::malformed_frame);
        }

        state_ = state::fin;
      }

      return event::payload::body{ id_, fin, std::move(r.value()) };
    }

    case state::fin:
      THROW(connection::error::internal);
  };

  if (sc == base::error::delegate) {
    frame frame = TRY(frame_.decode(buffers_));

    switch (frame) {
      case frame::type::headers:
        // TODO: Implement trailing HEADERS
        if (state_ == receiver::state::body) {
          LOG_W("Ignoring trailing headers");
          buffers_.consume(buffers_.size());
          state_ = state::fin;
          return event::payload::body{ id_, true, base::buffer() };
        }
        break;
      case frame::type::data:
        THROW(connection::error::unexpected_frame);
      case frame::type::settings:
      case frame::type::max_push_id:
      case frame::type::cancel_push:
      case frame::type::goaway:
        THROW(connection::error::wrong_stream);
      default:
        return process(frame);
    }
  }

  return sc;
}

}
}
}
}
}
