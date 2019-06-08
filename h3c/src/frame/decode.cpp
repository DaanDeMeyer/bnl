#include <h3c/frame.hpp>

#include <util/enum.hpp>
#include <util/error.hpp>

namespace h3c {

frame::decoder::decoder(logger *logger) noexcept
    : logger_(logger), varint_(logger)
{}

#define TRY_VARINT_DECODE(value)                                               \
  {                                                                            \
    const uint8_t *before = encoded.data();                                    \
    (value) = DECODE_TRY(varint_.decode(encoded, ec));                         \
                                                                               \
    size_t varint_encoded_size = static_cast<size_t>(encoded.data() - before); \
                                                                               \
    if (varint_encoded_size > payload_encoded_size) {                          \
      LOG_E("Frame payload's actual length exceeds its advertised length");    \
      DECODE_THROW(error::malformed_frame);                                    \
    }                                                                          \
                                                                               \
    payload_encoded_size -= varint_encoded_size;                               \
  }                                                                            \
  (void) 0

#define TRY_UINT8_DECODE(value)                                                \
  if (encoded.empty()) {                                                       \
    DECODE_THROW(error::incomplete);                                           \
  }                                                                            \
                                                                               \
  if (payload_encoded_size == 0) {                                             \
    LOG_E("Frame payload's actual length exceeds its advertised length");      \
    DECODE_THROW(error::malformed_frame);                                      \
  }                                                                            \
                                                                               \
  (value) = *encoded;                                                          \
                                                                               \
  encoded.advance(1);                                                          \
  payload_encoded_size--;                                                      \
  (void) 0

#define TRY_SETTING_DECODE(setting, value)                                     \
  {                                                                            \
    uint64_t encoded_value = 0;                                                \
    TRY_VARINT_DECODE(encoded_value);                                          \
                                                                               \
    if (encoded_value > setting::max) {                                        \
      LOG_E("Value of {} ({}) exceeds maximum ({})", #setting, encoded_value,  \
            setting::max);                                                     \
      DECODE_THROW(error::malformed_frame);                                    \
    }                                                                          \
                                                                               \
    (value) = static_cast<decltype(value)>(encoded_value);                     \
  }                                                                            \
  (void) 0

frame::type
frame::decoder::peek(buffer &encoded, std::error_code &ec) const noexcept
{
  DECODE_START();

  while (true) {
    uint64_t type = DECODE_TRY(varint_.decode(encoded, ec));

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
        DECODE_RESET();
        return static_cast<frame::type>(type);
      default:
        break;
    }
  }

  NOTREACHED();
}

frame frame::decoder::decode(buffer &encoded, std::error_code &ec) const
    noexcept
{
  while (true) {
    DECODE_START();

    uint64_t type = DECODE_TRY(varint_.decode(encoded, ec));
    uint64_t payload_encoded_size = DECODE_TRY(varint_.decode(encoded, ec));

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

          uint8_t byte = 0;
          TRY_UINT8_DECODE(byte);
          priority.prioritized_element_type =
              static_cast<frame::payload::priority::type>(byte >> 6U);
          priority.element_dependency_type =
              static_cast<frame::payload::priority::type>(
                  static_cast<uint8_t>(byte >> 4U) & 0x03U);

          TRY_VARINT_DECODE(priority.prioritized_element_id);
          TRY_VARINT_DECODE(priority.element_dependency_id);

          TRY_UINT8_DECODE(priority.weight);

          return priority;
        }

        case util::to_underlying(frame::type::cancel_push): {
          frame::payload::cancel_push cancel_push{};

          TRY_VARINT_DECODE(cancel_push.push_id);

          return cancel_push;
        }

        case util::to_underlying(frame::type::settings): {
          frame::payload::settings settings{};

          while (payload_encoded_size > 0) {
            uint64_t id = 0;
            TRY_VARINT_DECODE(id);

            switch (id) {
              case setting::max_header_list_size::id:
                TRY_SETTING_DECODE(setting::max_header_list_size,
                                   settings.max_header_list_size);
                break;
              case setting::num_placeholders::id:
                TRY_SETTING_DECODE(setting::num_placeholders,
                                   settings.num_placeholders);
                break;
              case setting::qpack_max_table_capacity::id:
                TRY_SETTING_DECODE(setting::qpack_max_table_capacity,
                                   settings.qpack_max_table_capacity);
                break;
              case setting::qpack_blocked_streams::id:
                TRY_SETTING_DECODE(setting::qpack_blocked_streams,
                                   settings.qpack_blocked_streams);
                break;
              default:;
                // Unknown setting id => ignore its value
                uint64_t value = 0;
                TRY_VARINT_DECODE(value);
                (void) value;
            }
          }

          return settings;
        }

        case util::to_underlying(frame::type::push_promise): {
          frame::payload::push_promise push_promise{};

          TRY_VARINT_DECODE(push_promise.push_id);

          push_promise.size = payload_encoded_size;
          payload_encoded_size = 0;

          return push_promise;
        }

        case util::to_underlying(frame::type::goaway): {
          frame::payload::goaway goaway{};

          TRY_VARINT_DECODE(goaway.stream_id);

          return goaway;
        }

        case util::to_underlying(frame::type::max_push_id): {
          frame::payload::max_push_id max_push_id{};

          TRY_VARINT_DECODE(max_push_id.push_id);

          return max_push_id;
        }

        case util::to_underlying(frame::type::duplicate_push): {
          frame::payload::duplicate_push duplicate_push{};

          TRY_VARINT_DECODE(duplicate_push.push_id);

          return duplicate_push;
        }

        default:
          // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#extensions
          // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-grease
          LOG_E("Ignoring unknown frame type ({})", type);
          is_unknown_frame_type = true;

          // TODO: Error on unreasonable unknown frame payload size.
          encoded.advance(static_cast<size_t>(payload_encoded_size));

          payload_encoded_size = 0;
          return {};
      }
    };

    frame frame = DECODE_TRY(payload_decode());

    if (payload_encoded_size > 0) {
      LOG_E("Frame payload's advertised length exceeds its actual length");
      DECODE_THROW(error::malformed_frame);
    }

    if (!is_unknown_frame_type) {
      return frame;
    }
  }

  NOTREACHED();
}

} // namespace h3c
