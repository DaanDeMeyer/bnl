#include <bnl/http3/codec/body.hpp>

namespace bnl {
namespace http3 {
namespace body {

result<void>
encoder::add(base::buffer body)
{
  if (fin_) {
    return error::internal;
  }

  buffers_.push(std::move(body));

  return base::success();
}

result<void>
encoder::fin() noexcept
{
  if (state_ == state::fin) {
    return error::internal;
  }

  fin_ = true;

  if (buffers_.empty()) {
    state_ = state::fin;
  }

  return base::success();
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
        return error::idle;
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
      return error::internal;
  }

  return error::internal;
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
        return error::delegate;
      }

      frame frame = BNL_TRY(frame::decode(encoded));

      state_ = state::data;
      remaining_ = frame.data.size;
    }
    /* FALLTHRU */
    case state::data: {
      if (encoded.empty()) {
        return error::incomplete;
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

  return error::internal;
}

BNL_BASE_SEQUENCE_IMPL(BNL_HTTP3_BODY_DECODE_IMPL);

}
}
}
