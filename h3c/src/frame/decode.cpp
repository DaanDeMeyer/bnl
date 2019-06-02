#include <h3c/frame.hpp>

#include <h3c/varint.hpp>

#include <util/enum.hpp>
#include <util/error.hpp>

#include <cassert>
#include <limits>

namespace h3c {

frame::decoder::decoder(logger *logger) noexcept
    : logger_(logger), varint_(logger)
{}

#define TRY_VARINT_DECODE_1(value)                                             \
  {                                                                            \
    size_t varint_encoded_size = 0;                                            \
    TRY(varint_.decode(src, size, &(value), &varint_encoded_size));            \
                                                                               \
    src += varint_encoded_size;                                                \
    size -= varint_encoded_size;                                               \
  }                                                                            \
  (void) 0

// Additionally checks and updates `frame_length` compared to
// `TRY_VARINT_DECODE_1`.
#define TRY_VARINT_DECODE_2(value)                                             \
  {                                                                            \
    size_t varint_encoded_size = 0;                                            \
    TRY(varint_.decode(src, size, &(value), &varint_encoded_size));            \
                                                                               \
    if (varint_encoded_size > payload_encoded_size) {                          \
      H3C_LOG_ERROR(                                                           \
          logger_,                                                             \
          "Frame payload's actual length exceeds its advertised length");      \
      THROW(error::malformed_frame);                                           \
    }                                                                          \
                                                                               \
    src += varint_encoded_size;                                                \
    size -= varint_encoded_size;                                               \
    payload_encoded_size -= varint_encoded_size;                               \
  }                                                                            \
  (void) 0

#define TRY_UINT8_DECODE(value)                                                \
  if (size == 0) {                                                             \
    THROW(error::incomplete);                                                  \
  }                                                                            \
                                                                               \
  if (payload_encoded_size == 0) {                                             \
    H3C_LOG_ERROR(                                                             \
        logger_,                                                               \
        "Frame payload's actual length exceeds its advertised length");        \
    THROW(error::malformed_frame);                                             \
  }                                                                            \
                                                                               \
  (value) = *src;                                                              \
                                                                               \
  src++;                                                                       \
  size--;                                                                      \
  payload_encoded_size--;                                                      \
  (void) 0

#define TRY_SETTING_DECODE(setting, value)                                     \
  {                                                                            \
    uint64_t encoded_value = 0;                                                \
    TRY_VARINT_DECODE_2(encoded_value);                                        \
                                                                               \
    if (encoded_value > setting::max) {                                        \
      H3C_LOG_ERROR(logger_, "Value of {} ({}) exceeds maximum ({})",          \
                    #setting, encoded_value, setting::max);                    \
      THROW(error::malformed_frame);                                           \
    }                                                                          \
                                                                               \
    (value) = static_cast<decltype(value)>(encoded_value);                     \
  }                                                                            \
  (void) 0

std::error_code frame::decoder::decode(const uint8_t *src,
                                       size_t size,
                                       frame *frame,
                                       size_t *encoded_size) const noexcept
{
  assert(src);
  assert(frame);
  assert(encoded_size);

  *encoded_size = 0;
  const uint8_t *begin = src;

  uint64_t type = 0;
  TRY_VARINT_DECODE_1(type);

  uint64_t payload_encoded_size = 0;
  TRY_VARINT_DECODE_1(payload_encoded_size);

  switch (type) {

    case util::to_underlying(frame::type::data): {
      frame::payload::data data{};

      data.size = payload_encoded_size;
      payload_encoded_size = 0;

      *frame = data;
      break;
    }

    case util::to_underlying(frame::type::headers): {
      frame::payload::headers headers{};

      headers.size = payload_encoded_size;
      payload_encoded_size = 0;

      *frame = headers;
      break;
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

      TRY_VARINT_DECODE_2(priority.prioritized_element_id);
      TRY_VARINT_DECODE_2(priority.element_dependency_id);

      TRY_UINT8_DECODE(priority.weight);

      *frame = priority;
      break;
    }

    case util::to_underlying(frame::type::cancel_push): {
      frame::payload::cancel_push cancel_push{};

      TRY_VARINT_DECODE_2(cancel_push.push_id);

      *frame = cancel_push;
      break;
    }

    case util::to_underlying(frame::type::settings): {
      frame::payload::settings settings{};

      while (payload_encoded_size > 0) {
        uint64_t id = 0;
        TRY_VARINT_DECODE_2(id);

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
            TRY_VARINT_DECODE_2(value);
        }
      }

      *frame = settings;
      break;
    }

    case util::to_underlying(frame::type::push_promise): {
      frame::payload::push_promise push_promise{};

      TRY_VARINT_DECODE_2(push_promise.push_id);

      push_promise.size = payload_encoded_size;
      payload_encoded_size = 0;

      *frame = push_promise;
      break;
    }

    case util::to_underlying(frame::type::goaway): {
      frame::payload::goaway goaway{};

      TRY_VARINT_DECODE_2(goaway.stream_id);

      *frame = goaway;
      break;
    }

    case util::to_underlying(frame::type::max_push_id): {
      frame::payload::max_push_id max_push_id{};

      TRY_VARINT_DECODE_2(max_push_id.push_id);

      *frame = max_push_id;
      break;
    }

    case util::to_underlying(frame::type::duplicate_push): {
      frame::payload::duplicate_push duplicate_push{};

      TRY_VARINT_DECODE_2(duplicate_push.push_id);

      *frame = duplicate_push;
      break;
    }

    default:
      THROW(error::unknown_frame_type);
  }

  if (payload_encoded_size > 0) {
    H3C_LOG_ERROR(
        logger_, "Frame payload's advertised length exceeds its actual length");
    THROW(error::malformed_frame);
  }

  *encoded_size = static_cast<size_t>(src - begin);

  return {};
}

} // namespace h3c
