#include <bnl/http3/endpoint/stream/request.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/log.hpp>

namespace bnl {
namespace http3 {
namespace endpoint {
namespace stream {
namespace request {

sender::sender(uint64_t id) noexcept
  : id_(id)
{}

sender::sender(sender &&other) noexcept
  : handle_(other.handle_)
  , state_(other.state_)
  , headers_(std::move(other.headers_))
  , body_(std::move(other.body_))
  , id_(other.id_)
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
      base::buffer encoded = BNL_TRY(headers_.encode());

      if (headers_.finished()) {
        state_ = body_.finished() ? state::fin : state::body;
      }

      bool fin = headers_.finished() && body_.finished();

      return quic::data{ id_, fin, std::move(encoded) };
    }

    case state::body: {
      base::buffer encoded = BNL_TRY(body_.encode());

      if (body_.finished()) {
        state_ = state::fin;
      }

      return quic::data{ id_, body_.finished(), std::move(encoded) };
    }

    case state::fin:
      return connection::error::internal;
  }

  assert(false);
  return connection::error::internal;
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

receiver::receiver(uint64_t id) noexcept
  : id_(id)
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

  return success();
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
    return connection::error::internal;
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
        return success();
      }

      return std::move(r).error();
    }

    BNL_TRY(handler(std::move(r).value()));
  }

  return success();
}

result<event>
receiver::process() noexcept
{
  result<void>::error_type sc;

  switch (state_) {

    case state::closed:
      return connection::error::internal;

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
          return (connection::error::malformed_frame);
        }

        state_ = state::fin;
      }

      return event::payload::body{ id_, fin, std::move(r.value()) };
    }

    case state::fin:
      return connection::error::internal;
  };

  if (sc == base::error::delegate) {
    frame frame = BNL_TRY(frame::decode(buffers_));

    switch (frame) {
      case frame::type::headers:
        // TODO: Implement trailing HEADERS
        if (state_ == receiver::state::body) {
          BNL_LOG_W("Ignoring trailing headers");
          buffers_.consume(buffers_.size());
          state_ = state::fin;
          return event::payload::body{ id_, true, base::buffer() };
        }
        break;
      case frame::type::data:
        return connection::error::unexpected_frame;
      case frame::type::settings:
      case frame::type::max_push_id:
      case frame::type::cancel_push:
      case frame::type::goaway:
        return connection::error::wrong_stream;
      default:
        return process(frame);
    }
  }

  return failure(std::move(sc));
}

}
}
}
}
}
