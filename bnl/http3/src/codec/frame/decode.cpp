#include <bnl/http3/codec/frame.hpp>

#include <bnl/http3/error.hpp>
#include <bnl/util/enum.hpp>
#include <bnl/util/error.hpp>

#include <bnl/base/error.hpp>

namespace bnl {
namespace http3 {

frame::decoder::decoder(const log::api *logger) noexcept
    : logger_(logger), varint_(logger)
{}

template <typename Sequence>
frame::type frame::decoder::peek(const Sequence &encoded,
                                 std::error_code &ec) const noexcept
{
  typename Sequence::lookahead_type lookahead(encoded);

  while (true) {
    uint64_t type = TRY(varint_.decode(lookahead, ec));

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

template <typename Sequence>
frame frame::decoder::decode(Sequence &encoded, std::error_code &ec) const
    noexcept
{
  // frame has no copy constructor so we check the while condition inside the
  // while loop instead.
  while (true) {
    bool is_unknown_frame_type = false;
    typename Sequence::lookahead_type lookahead(encoded);

    frame frame = TRY(decode(lookahead, &is_unknown_frame_type, ec));
    encoded.consume(lookahead.consumed());

    if (!is_unknown_frame_type) {
      return frame;
    }
  }
}

template <typename Lookahead>
frame frame::decoder::decode(Lookahead &lookahead,
                             bool *is_unknown_frame_type,
                             std::error_code &ec) const noexcept
{
  uint64_t type = TRY(varint_.decode(lookahead, ec));
  uint64_t payload_encoded_size = TRY(varint_.decode(lookahead, ec));

  // Use lambda to get around lack of copy assignment operator on `frame`.
  auto payload_decode = [&]() -> frame {
    switch (static_cast<frame::type>(type)) {
      case frame::type::data: {
        frame::payload::data data{};

        data.size = payload_encoded_size;
        payload_encoded_size = 0;

        return data;
      }

      case frame::type::headers: {
        frame::payload::headers headers{};

        headers.size = payload_encoded_size;
        payload_encoded_size = 0;

        return headers;
      }

      case frame::type::priority: {
        frame::payload::priority priority{};

        uint8_t byte = TRY(uint8_decode(lookahead, ec));
        priority.prioritized_element_type =
            static_cast<frame::payload::priority::type>(byte >> 6U);
        priority.element_dependency_type =
            static_cast<frame::payload::priority::type>(
                static_cast<uint8_t>(byte >> 4U) & 0x03U);

        priority.prioritized_element_id = TRY(varint_.decode(lookahead, ec));
        priority.element_dependency_id = TRY(varint_.decode(lookahead, ec));

        priority.weight = TRY(uint8_decode(lookahead, ec));

        return priority;
      }

      case frame::type::cancel_push: {
        frame::payload::cancel_push cancel_push{};

        cancel_push.push_id = TRY(varint_.decode(lookahead, ec));

        return cancel_push;
      }

      case frame::type::settings: {
        frame::payload::settings settings{};

        size_t settings_encoded_size = 0;

        while (settings_encoded_size < payload_encoded_size) {
          size_t before = lookahead.consumed();
          uint64_t id = TRY(varint_.decode(lookahead, ec));
          uint64_t value = TRY(varint_.decode(lookahead, ec));

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

        return settings;
      }

      case frame::type::push_promise: {
        frame::payload::push_promise push_promise{};

        size_t before = lookahead.consumed();
        push_promise.push_id = TRY(varint_.decode(lookahead, ec));

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

        return push_promise;
      }

      case frame::type::goaway: {
        frame::payload::goaway goaway{};

        goaway.stream_id = TRY(varint_.decode(lookahead, ec));

        return goaway;
      }

      case frame::type::max_push_id: {
        frame::payload::max_push_id max_push_id{};

        max_push_id.push_id = TRY(varint_.decode(lookahead, ec));

        return max_push_id;
      }

      case frame::type::duplicate_push: {
        frame::payload::duplicate_push duplicate_push{};

        duplicate_push.push_id = TRY(varint_.decode(lookahead, ec));

        return duplicate_push;
      }

      default:
        // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#extensions
        // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-grease
        LOG_E("Ignoring unknown frame type ({})", type);
        *is_unknown_frame_type = true;

        // TODO: Error on unreasonable unknown frame payload size.
        lookahead.consume(static_cast<size_t>(payload_encoded_size));

        payload_encoded_size = 0;
        return {};
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

template <typename Lookahead>
uint8_t frame::decoder::uint8_decode(Lookahead &lookahead,
                                     std::error_code &ec) const noexcept
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
