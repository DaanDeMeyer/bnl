#include <bnl/http3/codec/frame.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/enum.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {

frame::decoder::decoder(const log::api *logger) noexcept
    : logger_(logger), varint_(logger)
{}

frame::type
frame::decoder::peek(buffer &encoded, std::error_code &ec) const noexcept
{
  return peek<buffer>(encoded, ec);
}

frame::type
frame::decoder::peek(buffers &encoded, std::error_code &ec) const noexcept
{
  return peek<buffers>(encoded, ec);
}

template <typename Sequence>
frame::type
frame::decoder::peek(Sequence &encoded, std::error_code &ec) const noexcept
{
  typename Sequence::anchor anchor(encoded);

  while (true) {
    uint64_t type = TRY(varint_.decode(encoded, ec));

    switch (type) {
      case util::to_underlying(frame::type::data):
      case util::to_underlying(frame::type::headers):
      case util::to_underlying(frame::type::priority):
      case util::to_underlying(frame::type::cancel_push):
      case util::to_underlying(frame::type::settings):
      case util::to_underlying(frame::type::push_promise):
      case util::to_underlying(frame::type::goaway):
      case util::to_underlying(frame::type::max_push_id):
      case util::to_underlying(frame::type::duplicate_push):
        return static_cast<frame::type>(type);
      default:
        continue;
    }
  }

  NOTREACHED();
}

frame frame::decoder::decode(buffer &encoded, std::error_code &ec) const
    noexcept
{
  return decode<buffer>(encoded, ec);
}

frame frame::decoder::decode(buffers &encoded, std::error_code &ec) const
    noexcept
{
  return decode<buffers>(encoded, ec);
}

template <typename Sequence>
uint8_t
frame::decoder::uint8_decode(Sequence &encoded, std::error_code &ec) const
    noexcept
{
  CHECK(!encoded.empty(), error::incomplete);

  uint8_t result = *encoded;

  encoded += 1;

  return result;
}

template <typename Sequence>
frame frame::decoder::decode(Sequence &encoded, std::error_code &ec) const
    noexcept
{
  typename Sequence::anchor anchor(encoded);

  while (true) {
    uint64_t type = TRY(varint_.decode(encoded, ec));
    uint64_t payload_encoded_size = TRY(varint_.decode(encoded, ec));

    bool is_unknown_frame_type = false;

    // Use lambda to get around lack of copy assignment operator on `frame`.
    auto payload_decode = [&]() -> frame {
      switch (type) {
        case util::to_underlying(frame::type::data): {
          frame::payload::data data{};

          data.size = payload_encoded_size;
          payload_encoded_size = 0;

          return data;
        }

        case util::to_underlying(frame::type::headers): {
          frame::payload::headers headers{};

          headers.size = payload_encoded_size;
          payload_encoded_size = 0;

          return headers;
        }

        case util::to_underlying(frame::type::priority): {
          frame::payload::priority priority{};

          uint8_t byte = TRY(uint8_decode(encoded, ec));
          priority.prioritized_element_type =
              static_cast<frame::payload::priority::type>(byte >> 6U);
          priority.element_dependency_type =
              static_cast<frame::payload::priority::type>(
                  static_cast<uint8_t>(byte >> 4U) & 0x03U);

          priority.prioritized_element_id = TRY(varint_.decode(encoded, ec));
          priority.element_dependency_id = TRY(varint_.decode(encoded, ec));

          priority.weight = TRY(uint8_decode(encoded, ec));

          return priority;
        }

        case util::to_underlying(frame::type::cancel_push): {
          frame::payload::cancel_push cancel_push{};

          cancel_push.push_id = TRY(varint_.decode(encoded, ec));

          return cancel_push;
        }

        case util::to_underlying(frame::type::settings): {
          frame::payload::settings settings{};

          size_t settings_encoded_size = 0;

          while (settings_encoded_size < payload_encoded_size) {
            size_t before = encoded.consumed();

            uint64_t id = TRY(varint_.decode(encoded, ec));
            uint64_t value = TRY(varint_.decode(encoded, ec));

            settings_encoded_size += encoded.consumed() - before;

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

        case util::to_underlying(frame::type::push_promise): {
          frame::payload::push_promise push_promise{};

          size_t before = encoded.consumed();
          push_promise.push_id = TRY(varint_.decode(encoded, ec));
          size_t varint_encoded_size = encoded.consumed() - before;

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

        case util::to_underlying(frame::type::goaway): {
          frame::payload::goaway goaway{};

          goaway.stream_id = TRY(varint_.decode(encoded, ec));

          return goaway;
        }

        case util::to_underlying(frame::type::max_push_id): {
          frame::payload::max_push_id max_push_id{};

          max_push_id.push_id = TRY(varint_.decode(encoded, ec));

          return max_push_id;
        }

        case util::to_underlying(frame::type::duplicate_push): {
          frame::payload::duplicate_push duplicate_push{};

          duplicate_push.push_id = TRY(varint_.decode(encoded, ec));

          return duplicate_push;
        }

        default:
          // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#extensions
          // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-grease
          LOG_E("Ignoring unknown frame type ({})", type);
          is_unknown_frame_type = true;

          // TODO: Error on unreasonable unknown frame payload size.
          encoded += static_cast<size_t>(payload_encoded_size);

          payload_encoded_size = 0;
          return {};
      }
    };

    size_t before = encoded.consumed();

    frame frame = TRY(payload_decode());

    size_t actual_encoded_size = encoded.consumed() - before;

    CHECK_MSG(
        actual_encoded_size == payload_encoded_size, error::malformed_frame,
        "Frame payload's actual length does not match its advertised length");

    if (!is_unknown_frame_type) {
      anchor.release();
      return frame;
    }

    anchor.relocate();
  }

  NOTREACHED();
}

} // namespace http3
} // namespace bnl
