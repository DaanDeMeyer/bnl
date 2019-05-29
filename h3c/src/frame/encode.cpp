#include <h3c/frame.hpp>

#include <h3c/varint.hpp>

#include <util/error.hpp>

#include <cassert>

namespace h3c {

#define TRY_VARINT_SIZE(value)                                                 \
  {                                                                            \
    size_t varint_encoded_size = varint::encoded_size((value));                \
    /* Use the maximum varint size if the varint overflows. We return an       \
     * overflow error later when the frame is actually encoded. */             \
    varint_encoded_size = varint_encoded_size != 0 ? varint_encoded_size       \
                                                   : sizeof(uint64_t);         \
    payload_size += varint_encoded_size;                                       \
  }                                                                            \
  (void) 0

#define TRY_SETTING_SIZE(id, value)                                            \
  TRY_VARINT_SIZE((id));                                                       \
  TRY_VARINT_SIZE((value));                                                    \
  (void) 0

static uint64_t frame_payload_size(const frame &frame)
{
  uint64_t payload_size = 0;

  switch (frame.type) {
    case frame::type::data:
      payload_size += frame.data.size;
      break;
    case frame::type::headers:
      payload_size += frame.headers.size;
      break;
    case frame::type::priority:
      payload_size++; // PT size + DT size + Empty size = 1 byte. See
                      // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-priority
      TRY_VARINT_SIZE(frame.priority.prioritized_element_id);
      TRY_VARINT_SIZE(frame.priority.element_dependency_id);
      payload_size++; // Weight
      break;
    case frame::type::cancel_push:
      TRY_VARINT_SIZE(frame.cancel_push.push_id);
      break;
    case frame::type::settings:
// GCC warns us when a setting max is the same as the max size for the setting's
// integer type which we choose to ignore since `TRY_SETTING_SIZE` has to work
// with any kind of maximum.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
      TRY_SETTING_SIZE(setting::max_header_list_size::id,
                       frame.settings.max_header_list_size);
      TRY_SETTING_SIZE(setting::num_placeholders::id,
                       frame.settings.num_placeholders);
      TRY_SETTING_SIZE(setting::qpack_max_table_capacity::id,
                       frame.settings.qpack_max_table_capacity);
      TRY_SETTING_SIZE(setting::qpack_blocked_streams::id,
                       frame.settings.qpack_blocked_streams);
#pragma GCC diagnostic pop
      break;
    case frame::type::push_promise:
      TRY_VARINT_SIZE(frame.push_promise.push_id);
      payload_size += frame.push_promise.headers.size;
      break;
    case frame::type::goaway:
      TRY_VARINT_SIZE(frame.goaway.stream_id);
      break;
    case frame::type::max_push_id:
      TRY_VARINT_SIZE(frame.max_push_id.push_id);
      break;
    case frame::type::duplicate_push:
      TRY_VARINT_SIZE(frame.duplicate_push.push_id);
      break;
  }

  return payload_size;
}

#define TRY_VARINT_ENCODE(value)                                               \
  {                                                                            \
    size_t varint_encoded_size = 0;                                            \
    std::error_code error = varint::encode(dest, size, (value),                \
                                           &varint_encoded_size, logger);      \
    if (error) {                                                               \
      return error;                                                            \
    }                                                                          \
                                                                               \
    dest += varint_encoded_size;                                               \
    size -= varint_encoded_size;                                               \
  }                                                                            \
  (void) 0

#define TRY_UINT8_ENCODE(value)                                                \
  if (size == 0) {                                                             \
    THROW(error::buffer_too_small);                                            \
  }                                                                            \
                                                                               \
  *dest = (value);                                                             \
                                                                               \
  dest++;                                                                      \
  size--;                                                                      \
  (void) 0

#define TRY_SETTING_ENCODE(setting, value)                                     \
  if ((value) > setting::max) {                                                \
    H3C_LOG_ERROR(logger, "Value of {} ({}) exceeds maximum ({})",             \
                  setting::id, value, setting::max);                           \
    THROW(error::setting_overflow);                                            \
  }                                                                            \
                                                                               \
  TRY_VARINT_ENCODE((setting::id));                                            \
  TRY_VARINT_ENCODE((value));                                                  \
  (void) 0

size_t frame::encoded_size(const frame &frame)
{
  uint64_t payload_size = frame_payload_size(frame);
  size_t payload_encoded_size = 0;

  // `payload_size` includes the size of the DATA frame payload and
  // HEADERS/PUSH_PROMISE header block but these are not encoded in
  // `h3c_frame_encode` (this is left to the user) so we calculate the encoded
  // payload size of HEADERS, DATA and PUSH_PROMISE frames manually and delegate
  // the rest to `frame_payload_size`.
  switch (frame.type) {
    case frame::type::headers:
    case frame::type::data:
      break;
    case frame::type::push_promise:
      payload_encoded_size = varint::encoded_size(frame.push_promise.push_id);
      break;
    default:
      // HEADERS, DATA and PUSH_PROMISE frames are the only frames that might
      // have a payload size exceeding a `size_t` value and those are handled
      // separately so the cast is safe.
      payload_encoded_size = static_cast<size_t>(payload_size);
      break;
  }

  size_t encoded_size = 0;

  encoded_size += varint::encoded_size(static_cast<uint64_t>(frame.type));
  encoded_size += varint::encoded_size(payload_size);
  encoded_size += payload_encoded_size;

  return encoded_size;
}

std::error_code frame::encode(uint8_t *dest,
                              size_t size,
                              const frame &frame,
                              size_t *encoded_size,
                              const logger *logger)
{
  assert(dest);
  assert(encoded_size);

  uint8_t *begin = dest;
  *encoded_size = 0;

  uint64_t payload_size = frame_payload_size(frame);

  TRY_VARINT_ENCODE(frame.type);
  TRY_VARINT_ENCODE(payload_size);

  switch (frame.type) {
    case frame::type::data:
      break;
    case frame::type::headers:
      break;
    case frame::type::priority: {
      uint8_t prioritized_element_type = static_cast<uint8_t>(
          frame.priority.prioritized_element_type);
      uint8_t element_dependency_type = static_cast<uint8_t>(
          frame.priority.element_dependency_type);

      uint8_t byte = 0;
      byte |= static_cast<uint8_t>((prioritized_element_type << 6U));
      byte |= static_cast<uint8_t>(element_dependency_type << 4U);
      byte &= 0xf0U;
      TRY_UINT8_ENCODE(byte);

      TRY_VARINT_ENCODE(frame.priority.prioritized_element_id);
      TRY_VARINT_ENCODE(frame.priority.element_dependency_id);

      TRY_UINT8_ENCODE(frame.priority.weight);
      break;
    }
    case frame::type::cancel_push:
      TRY_VARINT_ENCODE(frame.cancel_push.push_id);
      break;
    case frame::type::settings:
      TRY_SETTING_ENCODE(setting::max_header_list_size,
                         frame.settings.max_header_list_size);
      TRY_SETTING_ENCODE(setting::num_placeholders,
                         frame.settings.num_placeholders);
      TRY_SETTING_ENCODE(setting::qpack_max_table_capacity,
                         frame.settings.qpack_max_table_capacity);
      TRY_SETTING_ENCODE(setting::qpack_blocked_streams,
                         frame.settings.qpack_blocked_streams);
      break;
    case frame::type::push_promise:
      TRY_VARINT_ENCODE(frame.push_promise.push_id);
      break;
    case frame::type::goaway:
      TRY_VARINT_ENCODE(frame.goaway.stream_id);
      break;
    case frame::type::max_push_id:
      TRY_VARINT_ENCODE(frame.max_push_id.push_id);
      break;
    case frame::type::duplicate_push:
      TRY_VARINT_ENCODE(frame.duplicate_push.push_id);
      break;
  }

  *encoded_size = static_cast<size_t>(dest - begin);

  return {};
}

} // namespace h3c
