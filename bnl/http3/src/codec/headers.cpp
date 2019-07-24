#include <bnl/http3/codec/headers.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace headers {

encoder::encoder(const log::api *logger) noexcept
  : frame_(logger)
  , qpack_(logger)
  , logger_(logger)
{}

result<void>
encoder::add(header_view header)
{
  if (state_ != state::idle) {
    THROW(connection::error::internal);
  }

  base::buffer encoded = TRY(qpack_.encode(header));
  buffers_.push(std::move(encoded));

  return success();
}

result<void>
encoder::fin() noexcept
{
  if (state_ != state::idle) {
    THROW(connection::error::internal);
  }

  state_ = state::frame;

  return success();
}

bool
encoder::finished() const noexcept
{
  return state_ == state::fin;
}

result<base::buffer>
encoder::encode() noexcept
{
  switch (state_) {

    case state::idle:
      return base::error::idle;

    case state::frame: {
      frame frame = frame::payload::headers{ qpack_.count() };

      base::buffer encoded = TRY(frame_.encode(frame));

      state_ = state::qpack;

      return encoded;
    }

    case state::qpack: {
      base::buffer encoded = buffers_.pop();
      state_ = buffers_.empty() ? state::fin : state_;

      return encoded;
    }

    case state::fin:
      THROW(connection::error::internal);
  }

  NOTREACHED();
}

decoder::decoder(const log::api *logger) noexcept
  : frame_(logger)
  , qpack_(logger)
  , logger_(logger)
{}

bool
decoder::started() const noexcept
{
  return state_ != state::frame;
}

bool
decoder::finished() const noexcept
{
  return state_ == state::fin;
}

template<typename Sequence>
result<header>
decoder::decode(Sequence &encoded)
{
  switch (state_) {

    case state::frame: {
      frame::type type = TRY(frame_.peek(encoded));

      if (type != frame::type::headers) {
        return base::error::delegate;
      }

      frame frame = TRY(frame_.decode(encoded));

      state_ = state::qpack;
      headers_size_ = frame.headers.size;

      if (headers_size_ == 0) {
        THROW(connection::error::malformed_frame);
      }
    }
    /* FALLTHRU */
    case state::qpack: {
      header header = TRY(qpack_.decode(encoded));

      if (qpack_.count() > headers_size_) {
        THROW(connection::error::malformed_frame);
      }

      bool fin = qpack_.count() == headers_size_;
      state_ = fin ? state::fin : state_;

      return header;
    }

    case state::fin:
      THROW(connection::error::internal);
  }

  NOTREACHED();
}

BNL_BASE_SEQUENCE_IMPL(BNL_HTTP3_HEADERS_DECODE_IMPL);

}
}
}
