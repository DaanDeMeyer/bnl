#include <h3c/endpoint/stream/request.hpp>

#include <util/error.hpp>
#include <util/stream.hpp>

namespace h3c {
namespace stream {

request::handle::handle(uint64_t id,
                        request::encoder *ref,
                        logger *logger) noexcept
    : id_(id), ref_(ref), logger_(logger)
{}

request::handle::handle(handle &&other) noexcept : handle()
{
  operator=(std::move(other));
}

request::handle &request::handle::operator=(handle &&other) noexcept
{
  std::swap(ref_, other.ref_);

  if (ref_ != nullptr) {
    ref_->handle_ = this;
  }

  return *this;
}

request::handle::~handle() noexcept
{
  if (ref_ == nullptr) {
    return;
  }

  ref_->handle_ = nullptr;
}

bool request::handle::valid() const noexcept
{
  return ref_ != nullptr;
}

uint64_t request::handle::id() const noexcept
{
  return id_;
}

void request::handle::header(header_view header, std::error_code &ec)
{
  if (ref_ == nullptr) {
    THROW_VOID(error::stream_closed);
  }

  return ref_->headers_.add(header, ec);
}

void request::handle::start(std::error_code &ec) noexcept
{
  if (ref_ == nullptr) {
    THROW_VOID(error::stream_closed);
  }

  return ref_->headers_.fin(ec);
}

void request::handle::body(buffer body, std::error_code &ec)
{
  if (ref_ == nullptr) {
    THROW_VOID(error::stream_closed);
  }

  return ref_->body_.add(std::move(body), ec);
}

void request::handle::fin(std::error_code &ec) noexcept
{
  if (ref_ == nullptr) {
    THROW_VOID(error::stream_closed);
  }

  return ref_->body_.fin(ec);
}

request::encoder::encoder(uint64_t id, logger *logger) noexcept
    : id_(id), logger_(logger), headers_(id, logger), body_(id, logger)
{}

request::encoder::~encoder() noexcept
{
  if (handle_ == nullptr) {
    return;
  }

  handle_->ref_ = nullptr;
}

request::encoder::operator state() const noexcept
{
  return state_;
}

request::handle request::encoder::handle() noexcept
{
  request::handle result(id_, this, logger_);

  if (handle_ != nullptr) {
    // Invalidate previous handle.
    handle_->ref_ = nullptr;
  }

  handle_ = &result;

  return result;
}

quic::data request::encoder::encode(std::error_code &ec) noexcept
{
  switch (state_) {

    case state::headers: {
      quic::data data = STREAM_ENCODE_TRY(headers_.encode(ec));

      if (headers_ == headers::encoder::state::fin) {
        state_ = state::body;
      }

      return data;
    }

    case state::body: {
      quic::data data = STREAM_ENCODE_TRY(body_.encode(ec));

      if (body_ == body::encoder::state::fin) {
        state_ = state::fin;
      }

      return data;
    }

    case state::fin:
    case state::error:
      NOTREACHED();
  }

  NOTREACHED();
}

request::decoder::decoder(uint64_t id, logger *logger) noexcept
    : id_(id),
      logger_(logger),
      frame_(logger),
      headers_(id, logger),
      body_(id, logger)
{}

request::decoder::operator state() const noexcept
{
  return state_;
}

void request::decoder::start(std::error_code &ec) noexcept
{
  if (state_ != state::closed) {
    THROW_VOID(error::internal_error);
  }

  state_ = state::headers;
}

event request::decoder::decode(quic::data &data, std::error_code &ec) noexcept
{
  STREAM_DECODE_START();

  // Use lambda to get around lack of copy assignment operator on `event`.
  auto decode = [&]() -> event {
    switch (state_) {

      case state::closed:
        STREAM_DECODE_THROW(error::internal_error);

      case state::headers: {
        event event = STREAM_DECODE_TRY(headers_.decode(data, ec));

        bool empty = data.buffer.empty() && data.fin;

        if (headers_ == headers::decoder::state::fin && empty) {
          state_ = state::fin;
        } else if (headers_ == headers::decoder::state::fin) {
          state_ = state::body;
        }

        return event;
      }

      case state::body: {
        event event = STREAM_DECODE_TRY(body_.decode(data, ec));

        if (body_ == body::decoder::state::fin) {
          state_ = state::fin;
        }

        return event;
      }

      case state::fin:
      case state::error:
        NOTREACHED();
    };

    NOTREACHED();
  };

  event event = STREAM_DECODE_TRY(decode());

  if (ec == error::unknown) {
    frame::type type = STREAM_DECODE_TRY(frame_.peek(data.buffer, ec));

    switch (type) {
      case frame::type::headers:
        if (state_ == stream::request::decoder::state::body) {
          // TODO: Implement trailing HEADERS
          STREAM_DECODE_THROW(error::not_implemented);
        }
        break;
      case frame::type::data:
        STREAM_DECODE_THROW(error::unexpected_frame);
      case frame::type::settings:
      case frame::type::max_push_id:
      case frame::type::cancel_push:
      case frame::type::goaway:
        STREAM_DECODE_THROW(error::wrong_stream);
      default:
        break;
    }

    STREAM_DECODE_THROW(error::unknown);
  }

  return event;
}

} // namespace stream
} // namespace h3c
