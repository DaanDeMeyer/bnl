#include <bnl/http3/codec/headers.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace headers {

encoder::encoder(const log::api* logger) noexcept
  : frame_(logger)
  , qpack_(logger)
  , logger_(logger)
{}

std::error_code
encoder::add(header_view header)
{
  CHECK(state_ == state::idle, error::internal_error);

  base::buffer encoded = TRY(qpack_.encode(header));
  buffers_.emplace(std::move(encoded));

  return {};
}

std::error_code
encoder::fin() noexcept
{
  CHECK(state_ == state::idle, error::internal_error);

  state_ = state::frame;

  return {};
}

bool
encoder::finished() const noexcept
{
  return state_ == state::fin;
}

base::result<base::buffer>
encoder::encode() noexcept
{
  switch (state_) {

    case state::idle:
      THROW(base::error::idle);

    case state::frame: {
      frame frame = frame::payload::headers{ qpack_.count() };

      base::buffer encoded = TRY(frame_.encode(frame));

      state_ = state::qpack;

      return encoded;
    }

    case state::qpack: {
      base::buffer encoded = std::move(buffers_.front());
      buffers_.pop();

      state_ = buffers_.empty() ? state::fin : state_;

      return encoded;
    }

    case state::fin:
      THROW(error::internal_error);
  }

  NOTREACHED();
}

decoder::decoder(const log::api* logger) noexcept
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
base::result<header>
decoder::decode(Sequence& encoded)
{
  switch (state_) {

    case state::frame: {
      frame::type type = TRY(frame_.peek(encoded));

      CHECK(type == frame::type::headers, base::error::unknown);

      frame frame = TRY(frame_.decode(encoded));

      state_ = state::qpack;
      headers_size_ = frame.headers.size;

      CHECK(headers_size_ != 0, error::malformed_frame);
    }
    /* FALLTHRU */
    case state::qpack: {
      header header = TRY(qpack_.decode(encoded));

      CHECK(qpack_.count() <= headers_size_, error::malformed_frame);

      bool fin = qpack_.count() == headers_size_;
      state_ = fin ? state::fin : state_;

      return header;
    }

    case state::fin:
      THROW(error::internal_error);
  }

  NOTREACHED();
}

BNL_BASE_SEQUENCE_IMPL(BNL_HTTP3_HEADERS_DECODE_IMPL);

} // namespace headers
} // namespace http3
} // namespace bnl
