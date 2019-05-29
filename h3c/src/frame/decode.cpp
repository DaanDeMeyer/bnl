#include <h3c/frame.hpp>

#include <h3c/varint.hpp>

#include <util/error.hpp>

#include <cassert>
#include <limits>

namespace h3c {

#define TRY_VARINT_DECODE_1(value)                                             \
  {                                                                            \
    size_t varint_encoded_size = 0;                                            \
    std::error_code error = varint::decode(src, size, &(value),                \
                                           &varint_encoded_size, logger);      \
    if (error) {                                                               \
      return error;                                                            \
    }                                                                          \
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
    std::error_code error = varint::decode(src, size, &(value),                \
                                           &varint_encoded_size, logger);      \
    if (error) {                                                               \
      return error;                                                            \
    }                                                                          \
                                                                               \
    if (varint_encoded_size > payload_encoded_size) {                          \
      H3C_LOG_ERROR(                                                           \
          logger,                                                              \
          "Frame payload's actual length exceeds its advertised length");      \
      THROW(error::malformed_frame);                                           \
    }                                                                          \
                                                                               \
    src += varint_encoded_size;                                                \
    size -= varint_encoded_size;                                               \
    payload_encoded_size -= varint_encoded_size;                               \
  }                                                                            \
  (void) 0

#define BUFFER_DECODE(buffer)                                                  \
  (buffer).size = payload_encoded_size;                                        \
  payload_encoded_size -= payload_encoded_size;                                \
  (void) 0

#define TRY_UINT8_DECODE(value)                                                \
  if (size == 0) {                                                             \
    THROW(error::incomplete);                                                  \
  }                                                                            \
                                                                               \
  if (payload_encoded_size == 0) {                                             \
    H3C_LOG_ERROR(                                                             \
        logger,                                                                \
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
    uint64_t varint = 0;                                                       \
    TRY_VARINT_DECODE_2(varint);                                               \
                                                                               \
    if (varint > setting::max) {                                               \
      H3C_LOG_ERROR(logger, "Value of {} ({}) exceeds maximum ({})",           \
                    setting::id, value, setting::max);                         \
      THROW(error::malformed_frame);                                           \
    }                                                                          \
                                                                               \
    (value) = static_cast<decltype(value)>(varint);                            \
  }                                                                            \
  (void) 0

std::error_code frame::decode(const uint8_t *src,
                              size_t size,
                              frame *frame,
                              size_t *encoded_size,
                              const logger *logger)
{
  assert(src);
  assert(frame);
  assert(encoded_size);

  *encoded_size = 0;
  const uint8_t *begin = src;

  TRY_VARINT_DECODE_1(frame->type);

  uint64_t payload_encoded_size = 0;
  TRY_VARINT_DECODE_1(payload_encoded_size);

  switch (frame->type) {
    case frame::type::data:
      BUFFER_DECODE(frame->data);
      break;
    case frame::type::headers:
      BUFFER_DECODE(frame->headers);
      break;
    case frame::type::priority: {
      uint8_t byte = 0;
      TRY_UINT8_DECODE(byte);
      frame->priority.prioritized_element_type =
          static_cast<frame::payload::priority::type>(byte >> 6U);
      frame->priority.element_dependency_type =
          static_cast<frame::payload::priority::type>(
              static_cast<uint8_t>(byte >> 4U) & 0x03U);

      TRY_VARINT_DECODE_2(frame->priority.prioritized_element_id);
      TRY_VARINT_DECODE_2(frame->priority.element_dependency_id);

      TRY_UINT8_DECODE(frame->priority.weight);
      break;
    }
    case frame::type::cancel_push:
      TRY_VARINT_DECODE_2(frame->cancel_push.push_id);
      break;
    case frame::type::settings:
      frame->settings = settings::initial();

      while (payload_encoded_size > 0) {
        uint64_t id = 0;
        TRY_VARINT_DECODE_2(id);

        switch (id) {
          case setting::max_header_list_size::id:
            TRY_SETTING_DECODE(setting::max_header_list_size,
                               frame->settings.max_header_list_size);
            break;
          case setting::num_placeholders::id:
            TRY_SETTING_DECODE(setting::num_placeholders,
                               frame->settings.num_placeholders);
            break;
          case setting::qpack_max_table_capacity::id:
            TRY_SETTING_DECODE(setting::qpack_max_table_capacity,
                               frame->settings.qpack_max_table_capacity);
            break;
          case setting::qpack_blocked_streams::id:
            TRY_SETTING_DECODE(setting::qpack_blocked_streams,
                               frame->settings.qpack_blocked_streams);
            break;
          default:;
            // Unknown setting id => ignore its value
            uint64_t value = 0;
            TRY_VARINT_DECODE_2(value);
        }
      }
      break;
    case frame::type::push_promise:
      TRY_VARINT_DECODE_2(frame->push_promise.push_id);
      BUFFER_DECODE(frame->push_promise.headers);
      break;
    case frame::type::goaway:
      TRY_VARINT_DECODE_2(frame->goaway.stream_id);
      break;
    case frame::type::max_push_id:
      TRY_VARINT_DECODE_2(frame->max_push_id.push_id);
      break;
    case frame::type::duplicate_push:
      TRY_VARINT_DECODE_2(frame->duplicate_push.push_id);
      break;
  }

  if (payload_encoded_size > 0) {
    H3C_LOG_ERROR(
        logger, "Frame payload's advertised length exceeds its actual length");
    THROW(error::malformed_frame);
  }

  *encoded_size = static_cast<size_t>(src - begin);

  return {};
}

} // namespace h3c
