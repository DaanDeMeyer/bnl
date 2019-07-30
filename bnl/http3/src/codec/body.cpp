#include <bnl/http3/codec/body.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>

namespace bnl {
namespace http3 {
namespace body {

result<void>
encoder::add(base::buffer body)
{
  if (fin_) {
    return connection::error::internal;
  }

  buffers_.push(std::move(body));

  return success();
}

result<void>
encoder::fin() noexcept
{
  if (state_ == state::fin) {
    return connection::error::internal;
  }

  fin_ = true;

  if (buffers_.empty()) {
    state_ = state::fin;
  }

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
  // TODO: Implement PRIORITY

  switch (state_) {

    case state::frame: {
      if (buffers_.empty()) {
        return base::error::idle;
      }

      frame frame = frame::payload::data{ buffers_.front().size() };
      base::buffer encoded = BNL_TRY(frame::encode(frame));

      state_ = state::data;

      return encoded;
    }

    case state::data: {
      base::buffer body = buffers_.pop();
      state_ = fin_ && buffers_.empty() ? state::fin : state::frame;

      return body;
    }

    case state::fin:
      return connection::error::internal;
  }

  return connection::error::internal;
}

bool
decoder::in_progress() const noexcept
{
  return state_ == decoder::state::data;
}

template<typename Sequence>
result<base::buffer>
decoder::decode(Sequence &encoded)
{
  switch (state_) {

    case state::frame: {
      frame::type type = BNL_TRY(frame::peek(encoded));

      if (type != frame::type::data) {
        return base::error::delegate;
      }

      frame frame = BNL_TRY(frame::decode(encoded));

      state_ = state::data;
      remaining_ = frame.data.size;
    }
    /* FALLTHRU */
    case state::data: {
      if (encoded.empty()) {
        return base::error::incomplete;
      }

      size_t body_part_size = encoded.size() < remaining_
                                ? encoded.size()
                                : static_cast<size_t>(remaining_);
      base::buffer body_part = encoded.slice(body_part_size);

      remaining_ -= body_part_size;

      assert(encoded.empty() || remaining_ == 0);

      state_ = remaining_ == 0 ? state::frame : state_;

      return body_part;
    }
  }

  return connection::error::internal;
}

BNL_BASE_SEQUENCE_IMPL(BNL_HTTP3_BODY_DECODE_IMPL);

}
}
}
