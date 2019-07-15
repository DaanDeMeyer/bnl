#include <bnl/http3/codec/frame.hpp>

#include <bnl/http3/error.hpp>
#include <bnl/util/enum.hpp>
#include <bnl/util/error.hpp>

#include <bnl/base/error.hpp>

namespace bnl {
namespace http3 {

frame::decoder::decoder(const log::api *logger) noexcept
  : varint_(logger)
  , logger_(logger)
{
}

template<typename Sequence>
base::result<frame::type>
frame::decoder::peek(const Sequence &encoded) const noexcept
{
  typename Sequence::lookahead_type lookahead(encoded);

  while (true) {
    uint64_t type = TRY(varint_.decode(lookahead));

    switch (static_cast<frame::type>(type)) {
      case frame::type::data:
      case frame::type::headers:
      case frame::type::priority:
      case frame::type::cancel_push:
      case frame::type::settings:
      case frame::type::push_promise:
      case frame::type::goaway:
      case frame::type::max_push_id:
      case frame::type::duplicate_push:
        return static_cast<frame::type>(type);
      default:
        continue;
    }
  }

  NOTREACHED();
}

template<typename Sequence>
base::result<frame>
frame::decoder::decode(Sequence &encoded) const noexcept
{
  // frame has no copy constructor so we check the while condition inside the
  // while loop instead.
  while (true) {
    typename Sequence::lookahead_type lookahead(encoded);

    base::result<frame> result = decode_single(lookahead);
    if (!result && result.error() != base::error::unknown) {
      return result.error();
    }

    encoded.consume(lookahead.consumed());

    if (result) {
      return result;
    }
  }
}

template<typename Lookahead>
base::result<frame>
frame::decoder::decode_single(Lookahead &lookahead) const noexcept
{
  uint64_t type = TRY(varint_.decode(lookahead));
  uint64_t payload_encoded_size = TRY(varint_.decode(lookahead));

  // Use lambda to get around lack of copy assignment operator on `frame`.
  auto payload_decode = [&]() -> base::result<frame> {
    switch (static_cast<frame::type>(type)) {
      case frame::type::data: {
        frame::payload::data data{};

        data.size = payload_encoded_size;
        payload_encoded_size = 0;

        return frame(data);
      }

      case frame::type::headers: {
        frame::payload::headers headers{};

        headers.size = payload_encoded_size;
        payload_encoded_size = 0;

        return frame(headers);
      }

      case frame::type::priority: {
        frame::payload::priority priority{};

        uint8_t byte = TRY(uint8_decode(lookahead));
        priority.prioritized_element_type =
          static_cast<frame::payload::priority::type>(byte >> 6U);
        priority.element_dependency_type =
          static_cast<frame::payload::priority::type>(
            static_cast<uint8_t>(byte >> 4U) & 0x03U);

        priority.prioritized_element_id = TRY(varint_.decode(lookahead));
        priority.element_dependency_id = TRY(varint_.decode(lookahead));

        priority.weight = TRY(uint8_decode(lookahead));

        return frame(priority);
      }

      case frame::type::cancel_push: {
        frame::payload::cancel_push cancel_push{};

        cancel_push.push_id = TRY(varint_.decode(lookahead));

        return frame(cancel_push);
      }

      case frame::type::settings: {
        frame::payload::settings settings{};

        size_t settings_encoded_size = 0;

        while (settings_encoded_size < payload_encoded_size) {
          size_t before = lookahead.consumed();
          uint64_t id = TRY(varint_.decode(lookahead));
          uint64_t value = TRY(varint_.decode(lookahead));

          settings_encoded_size += lookahead.consumed() - before;

          switch (id) {
            case setting::max_header_list_size:
              settings.max_header_list_size = value;
              break;
            case setting::num_placeholders:
              settings.num_placeholders = value;
              break;
            case setting::qpack_max_table_capacity:
              settings.qpack_max_table_capacity = value;
              break;
            case setting::qpack_blocked_streams:
              settings.qpack_blocked_streams = value;
              break;
            default:
              // Unknown setting => ignore
              break;
          }
        }

        return frame(settings);
      }

      case frame::type::push_promise: {
        frame::payload::push_promise push_promise{};

        size_t before = lookahead.consumed();
        push_promise.push_id = TRY(varint_.decode(lookahead));

        size_t varint_encoded_size = lookahead.consumed() - before;

        if (varint_encoded_size > payload_encoded_size) {
          // If the varint encoded size exceeds the payload encoded size an
          // error will be thrown at the end of this method so we just set the
          // push promise size to zero.
          push_promise.size = 0;
        } else {
          push_promise.size = payload_encoded_size - varint_encoded_size;
          // Make the size check at the end of the method pass.
          payload_encoded_size = varint_encoded_size;
        }

        return frame(push_promise);
      }

      case frame::type::goaway: {
        frame::payload::goaway goaway{};

        goaway.stream_id = TRY(varint_.decode(lookahead));

        return frame(goaway);
      }

      case frame::type::max_push_id: {
        frame::payload::max_push_id max_push_id{};

        max_push_id.push_id = TRY(varint_.decode(lookahead));

        return frame(max_push_id);
      }

      case frame::type::duplicate_push: {
        frame::payload::duplicate_push duplicate_push{};

        duplicate_push.push_id = TRY(varint_.decode(lookahead));

        return frame(duplicate_push);
      }

      default:
        // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#extensions
        // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-grease

        // TODO: Error on unreasonable unknown frame payload size.
        lookahead.consume(static_cast<size_t>(payload_encoded_size));
        THROW(base::error::unknown);
    }
  };

  size_t before = lookahead.consumed();
  frame frame = TRY(payload_decode());

  size_t actual_encoded_size = lookahead.consumed() - before;

  if (actual_encoded_size != payload_encoded_size) {
    LOG_E("Frame payload's actual length does not match its advertised length");
    THROW(error::malformed_frame);
  }

  return frame;
}

template<typename Lookahead>
base::result<uint8_t>
frame::decoder::uint8_decode(Lookahead &lookahead) const noexcept
{
  CHECK(!lookahead.empty(), base::error::incomplete);

  uint8_t result = *lookahead;

  lookahead.consume(sizeof(uint8_t));

  return result;
}

BNL_BASE_SEQUENCE_IMPL(BNL_HTTP3_FRAME_PEEK_IMPL);
BNL_BASE_LOOKAHEAD_IMPL(BNL_HTTP3_FRAME_PEEK_IMPL);

BNL_BASE_SEQUENCE_IMPL(BNL_HTTP3_FRAME_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_IMPL(BNL_HTTP3_FRAME_DECODE_IMPL);

} // namespace http3
} // namespace bnl
