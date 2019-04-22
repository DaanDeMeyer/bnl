#include <h3c/frame.h>
#include <h3c/varint.h>

#include <assert.h>
#include <string.h>

const h3c_frame_settings_t h3c_frame_settings_default = {
  .max_header_list_size = H3C_VARINT_MAX, // varint max
  .num_placeholders = 0,
  .qpack_max_table_capacity = 0,
  .qpack_blocked_streams = 0
};

// Setting identifiers

#define SETTINGS_MAX_HEADER_LIST_SIZE 0x6U
#define SETTINGS_NUM_PLACEHOLDERS 0x9U
#define SETTINGS_QPACK_MAX_TABLE_CAPACITY 0x1U
#define SETTINGS_QPACK_BLOCKED_STREAMS 0x7U

// Setting limits

#define SETTINGS_MAX_HEADER_LIST_SIZE_MAX (H3C_VARINT_MAX)
#define SETTINGS_NUM_PLACEHOLDERS_MAX (H3C_VARINT_MAX)
#define SETTINGS_QPACK_MAX_TABLE_CAPACITY_MAX ((1U << 30) - 1)
#define SETTINGS_QPACK_BLOCKED_STREAMS_MAX ((1U << 16) - 1)

// We use a lot of macros in this file because after each serialize/parse, we
// have to check the result and update a lot of values. Macros provide us with a
// concise way to accomplish this. Using functions instead of macros doesn't
// work because we would still have to check the return value of the function
// each time we call it, where macros allow us to return directly from the
// parent function.

#define TRY_VARINT_SIZE(value)                                                 \
  {                                                                            \
    size_t rv = h3c_varint_size((value));                                      \
    if (rv == 0) {                                                             \
      return H3C_FRAME_SERIALIZE_VARINT_OVERFLOW;                              \
    }                                                                          \
                                                                               \
    *size += rv;                                                               \
  }                                                                            \
  (void) 0

#define TRY_SETTING_SIZE(id, value)                                            \
  if ((value) > id##_MAX) {                                                    \
    return H3C_FRAME_SERIALIZE_SETTING_OVERFLOW;                               \
  }                                                                            \
                                                                               \
  TRY_VARINT_SIZE((id));                                                       \
  TRY_VARINT_SIZE((value));                                                    \
  (void) 0

static H3C_FRAME_SERIALIZE_ERROR frame_payload_size(const h3c_frame_t *frame,
                                                    uint64_t *size)
{
  assert(frame);

  *size = 0;

  switch (frame->type) {
  case H3C_FRAME_DATA:
    *size += frame->data.payload.size;
    break;
  case H3C_FRAME_HEADERS:
    *size += frame->headers.header_block.size;
    break;
  case H3C_FRAME_PRIORITY:
    (*size)++; // PT size + DT size + Empty size = 1 byte. See
               // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-priority
    TRY_VARINT_SIZE(frame->priority.prioritized_element_id);
    TRY_VARINT_SIZE(frame->priority.element_dependency_id);
    (*size)++; // Weight
    break;
  case H3C_FRAME_CANCEL_PUSH:
    TRY_VARINT_SIZE(frame->cancel_push.push_id);
    break;
  case H3C_FRAME_SETTINGS:
// GCC warns us when a setting max is the same as the max size for the setting's
// integer type which we choose to ignore since `TRY_SETTING_SIZE` has to work
// with any kind of maximum.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    TRY_SETTING_SIZE(SETTINGS_MAX_HEADER_LIST_SIZE,
                     frame->settings.max_header_list_size);
    TRY_SETTING_SIZE(SETTINGS_NUM_PLACEHOLDERS,
                     frame->settings.num_placeholders);
    TRY_SETTING_SIZE(SETTINGS_QPACK_MAX_TABLE_CAPACITY,
                     frame->settings.qpack_max_table_capacity);
    TRY_SETTING_SIZE(SETTINGS_QPACK_BLOCKED_STREAMS,
                     frame->settings.qpack_blocked_streams);
#pragma GCC diagnostic pop
    break;
  case H3C_FRAME_PUSH_PROMISE:
    TRY_VARINT_SIZE(frame->push_promise.push_id);
    *size += frame->push_promise.header_block.size;
    break;
  case H3C_FRAME_GOAWAY:
    TRY_VARINT_SIZE(frame->goaway.stream_id);
    break;
  case H3C_FRAME_MAX_PUSH_ID:
    TRY_VARINT_SIZE(frame->max_push_id.push_id);
    break;
  case H3C_FRAME_DUPLICATE_PUSH:
    TRY_VARINT_SIZE(frame->duplicate_push.push_id);
    break;
  }

  if (*size > H3C_VARINT_MAX) {
    return H3C_FRAME_SERIALIZE_VARINT_OVERFLOW;
  }

  return H3C_FRAME_SERIALIZE_SUCCESS;
}

#define TRY_VARINT_SERIALIZE(value)                                            \
  {                                                                            \
    size_t rv = h3c_varint_serialize(dest, size, (value));                     \
    if (rv == 0) {                                                             \
      return H3C_FRAME_SERIALIZE_BUF_TOO_SMALL;                                \
    }                                                                          \
                                                                               \
    dest += rv;                                                                \
    size -= rv;                                                                \
    *bytes_written += rv;                                                      \
  }                                                                            \
  (void) 0

#define TRY_UINT8_SERIALIZE(value)                                             \
  if (size == 0) {                                                             \
    return H3C_FRAME_SERIALIZE_BUF_TOO_SMALL;                                  \
  }                                                                            \
                                                                               \
  *dest = (value);                                                             \
                                                                               \
  dest++;                                                                      \
  size--;                                                                      \
  (*bytes_written)++;                                                          \
  (void) 0

#define TRY_SETTING_SERIALIZE(id, value)                                       \
  TRY_VARINT_SERIALIZE((id));                                                  \
  TRY_VARINT_SERIALIZE((value));                                               \
  (void) 0

H3C_FRAME_SERIALIZE_ERROR h3c_frame_serialize(uint8_t *dest,
                                              size_t size,
                                              const h3c_frame_t *frame,
                                              size_t *bytes_written)
{
  assert(dest);
  assert(frame);
  assert(bytes_written);

  *bytes_written = 0;

  uint64_t frame_length = 0;
  H3C_FRAME_SERIALIZE_ERROR error = frame_payload_size(frame, &frame_length);
  if (error) {
    return error;
  }

  TRY_VARINT_SERIALIZE(frame->type);
  TRY_VARINT_SERIALIZE(frame_length);

  switch (frame->type) {
  case H3C_FRAME_DATA:
    break;
  case H3C_FRAME_HEADERS:
    break;
  case H3C_FRAME_PRIORITY:;
    uint8_t byte = 0;
    byte |= (uint8_t)(frame->priority.prioritized_element_type << 6);
    byte |= (uint8_t)(frame->priority.element_dependency_type << 4);
    byte &= 0xf0;
    TRY_UINT8_SERIALIZE(byte);

    TRY_VARINT_SERIALIZE(frame->priority.prioritized_element_id);
    TRY_VARINT_SERIALIZE(frame->priority.element_dependency_id);

    TRY_UINT8_SERIALIZE(frame->priority.weight);
    break;
  case H3C_FRAME_CANCEL_PUSH:
    TRY_VARINT_SERIALIZE(frame->cancel_push.push_id);
    break;
  case H3C_FRAME_SETTINGS:
    TRY_SETTING_SERIALIZE(SETTINGS_MAX_HEADER_LIST_SIZE,
                          frame->settings.max_header_list_size);
    TRY_SETTING_SERIALIZE(SETTINGS_NUM_PLACEHOLDERS,
                          frame->settings.num_placeholders);
    TRY_SETTING_SERIALIZE(SETTINGS_QPACK_MAX_TABLE_CAPACITY,
                          frame->settings.qpack_max_table_capacity);
    TRY_SETTING_SERIALIZE(SETTINGS_QPACK_BLOCKED_STREAMS,
                          frame->settings.qpack_blocked_streams);
    break;
  case H3C_FRAME_PUSH_PROMISE:
    TRY_VARINT_SERIALIZE(frame->push_promise.push_id);
    break;
  case H3C_FRAME_GOAWAY:
    TRY_VARINT_SERIALIZE(frame->goaway.stream_id);
    break;
  case H3C_FRAME_MAX_PUSH_ID:
    TRY_VARINT_SERIALIZE(frame->max_push_id.push_id);
    break;
  case H3C_FRAME_DUPLICATE_PUSH:
    TRY_VARINT_SERIALIZE(frame->duplicate_push.push_id);
    break;
  }

  return H3C_FRAME_SERIALIZE_SUCCESS;
}

#define TRY_VARINT_PARSE_1(value)                                              \
  {                                                                            \
    size_t rv = h3c_varint_parse(src, size, &(value));                         \
    if (rv == 0) {                                                             \
      return H3C_FRAME_PARSE_INCOMPLETE;                                       \
    }                                                                          \
                                                                               \
    src += rv;                                                                 \
    size -= rv;                                                                \
    *bytes_read += rv;                                                         \
  }                                                                            \
  (void) 0

// Additionally checks and updates `frame_length` compared to
// `TRY_VARINT_PARSE_1`.
#define TRY_VARINT_PARSE_2(value)                                              \
  {                                                                            \
    size_t rv = h3c_varint_parse(src, size, &(value));                         \
    if (rv == 0) {                                                             \
      return H3C_FRAME_PARSE_INCOMPLETE;                                       \
    }                                                                          \
                                                                               \
    if (rv > frame_length) {                                                   \
      return H3C_FRAME_PARSE_MALFORMED;                                        \
    }                                                                          \
                                                                               \
    src += rv;                                                                 \
    size -= rv;                                                                \
    *bytes_read += rv;                                                         \
    frame_length -= rv;                                                        \
  }                                                                            \
  (void) 0

#define BUFFER_PARSE(buffer)                                                   \
  (buffer).size = frame_length;                                                \
  frame_length -= frame_length;                                                \
  (void) 0

#define TRY_UINT8_PARSE(value)                                                 \
  if (size == 0) {                                                             \
    return H3C_FRAME_PARSE_INCOMPLETE;                                         \
  }                                                                            \
                                                                               \
  if (frame_length == 0) {                                                     \
    return H3C_FRAME_PARSE_MALFORMED;                                          \
  }                                                                            \
                                                                               \
  (value) = *src;                                                              \
                                                                               \
  src++;                                                                       \
  size--;                                                                      \
  (*bytes_read)++;                                                             \
  frame_length--;                                                              \
  (void) 0

#define TRY_SETTING_PARSE(id, value, type)                                     \
  {                                                                            \
    uint64_t varint = 0;                                                       \
    TRY_VARINT_PARSE_2(varint);                                                \
                                                                               \
    if (varint > id##_MAX) {                                                   \
      return H3C_FRAME_PARSE_MALFORMED;                                        \
    }                                                                          \
                                                                               \
    (value) = (type) varint;                                                   \
  }                                                                            \
  (void) 0

H3C_FRAME_PARSE_ERROR h3c_frame_parse(const uint8_t *src,
                                      size_t size,
                                      h3c_frame_t *frame,
                                      size_t *bytes_read)
{
  assert(src);
  assert(frame);
  assert(bytes_read);

  *bytes_read = 0;

  TRY_VARINT_PARSE_1(frame->type);

  uint64_t frame_length = 0;
  TRY_VARINT_PARSE_1(frame_length);

  switch (frame->type) {
  case H3C_FRAME_DATA:
    BUFFER_PARSE(frame->data.payload);
    break;
  case H3C_FRAME_HEADERS:
    BUFFER_PARSE(frame->headers.header_block);
    break;
  case H3C_FRAME_PRIORITY:;
    uint8_t byte = 0;
    TRY_UINT8_PARSE(byte);
    frame->priority.prioritized_element_type = byte >> 6;
    frame->priority.element_dependency_type = (byte >> 4) & 0x03;

    TRY_VARINT_PARSE_2(frame->priority.prioritized_element_id);
    TRY_VARINT_PARSE_2(frame->priority.element_dependency_id);

    TRY_UINT8_PARSE(frame->priority.weight);
    break;
  case H3C_FRAME_CANCEL_PUSH:
    TRY_VARINT_PARSE_2(frame->cancel_push.push_id);
    break;
  case H3C_FRAME_SETTINGS:
    frame->settings = h3c_frame_settings_default;

    while (frame_length > 0) {
      uint64_t id = 0;
      TRY_VARINT_PARSE_2(id);

      switch (id) {
      case SETTINGS_MAX_HEADER_LIST_SIZE:
        TRY_SETTING_PARSE(SETTINGS_MAX_HEADER_LIST_SIZE,
                          frame->settings.max_header_list_size, uint64_t);
        break;
      case SETTINGS_NUM_PLACEHOLDERS:
        TRY_SETTING_PARSE(SETTINGS_NUM_PLACEHOLDERS,
                          frame->settings.num_placeholders, uint64_t);
        break;
      case SETTINGS_QPACK_MAX_TABLE_CAPACITY:
        TRY_SETTING_PARSE(SETTINGS_QPACK_MAX_TABLE_CAPACITY,
                          frame->settings.qpack_max_table_capacity, uint32_t);
        break;
      case SETTINGS_QPACK_BLOCKED_STREAMS:
        TRY_SETTING_PARSE(SETTINGS_QPACK_BLOCKED_STREAMS,
                          frame->settings.qpack_blocked_streams, uint16_t);
        break;
      default:;
        // Unknown setting id => ignore its value
        uint64_t value = 0;
        TRY_VARINT_PARSE_2(value);
      }
    }
    break;
  case H3C_FRAME_PUSH_PROMISE:
    TRY_VARINT_PARSE_2(frame->push_promise.push_id);
    BUFFER_PARSE(frame->push_promise.header_block);
    break;
  case H3C_FRAME_GOAWAY:
    TRY_VARINT_PARSE_2(frame->goaway.stream_id);
    break;
  case H3C_FRAME_MAX_PUSH_ID:
    TRY_VARINT_PARSE_2(frame->max_push_id.push_id);
    break;
  case H3C_FRAME_DUPLICATE_PUSH:
    TRY_VARINT_PARSE_2(frame->duplicate_push.push_id);
    break;
  }

  if (frame_length > 0) {
    return H3C_FRAME_PARSE_MALFORMED;
  }

  return H3C_FRAME_PARSE_SUCCESS;
}
