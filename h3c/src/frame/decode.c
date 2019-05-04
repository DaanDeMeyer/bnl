#include <h3c/frame.h>

#include <assert.h>

#define TRY_VARINT_DECODE_1(value)                                             \
  {                                                                            \
    size_t varint_size = 0;                                                    \
    H3C_ERROR error = h3c_varint_decode(src, size, &(value), &varint_size);    \
    if (error) {                                                               \
      return H3C_ERROR_INCOMPLETE;                                             \
    }                                                                          \
                                                                               \
    src += varint_size;                                                        \
    size -= varint_size;                                                       \
    *frame_size += varint_size;                                                \
  }                                                                            \
  (void) 0

// Additionally checks and updates `frame_length` compared to
// `TRY_VARINT_DECODE_1`.
#define TRY_VARINT_DECODE_2(value)                                             \
  {                                                                            \
    size_t varint_size = 0;                                                    \
    H3C_ERROR error = h3c_varint_decode(src, size, &(value), &varint_size);    \
    if (error) {                                                               \
      return H3C_ERROR_INCOMPLETE;                                             \
    }                                                                          \
                                                                               \
    if (varint_size > payload_size) {                                          \
      return H3C_ERROR_MALFORMED_FRAME;                                        \
    }                                                                          \
                                                                               \
    src += varint_size;                                                        \
    size -= varint_size;                                                       \
    *frame_size += varint_size;                                                \
    payload_size -= varint_size;                                               \
  }                                                                            \
  (void) 0

#define BUFFER_DECODE(buffer)                                                  \
  (buffer).size = payload_size;                                                \
  payload_size -= payload_size;                                                \
  (void) 0

#define TRY_UINT8_DECODE(value)                                                \
  if (size == 0) {                                                             \
    return H3C_ERROR_INCOMPLETE;                                               \
  }                                                                            \
                                                                               \
  if (payload_size == 0) {                                                     \
    return H3C_ERROR_MALFORMED_FRAME;                                          \
  }                                                                            \
                                                                               \
  (value) = *src;                                                              \
                                                                               \
  src++;                                                                       \
  size--;                                                                      \
  (*frame_size)++;                                                             \
  payload_size--;                                                              \
  (void) 0

#define TRY_SETTING_DECODE(id, value, type)                                    \
  {                                                                            \
    uint64_t varint = 0;                                                       \
    TRY_VARINT_DECODE_2(varint);                                               \
                                                                               \
    if (varint > id##_MAX) {                                                   \
      return H3C_ERROR_MALFORMED_FRAME;                                        \
    }                                                                          \
                                                                               \
    (value) = (type) varint;                                                   \
  }                                                                            \
  (void) 0

H3C_ERROR h3c_frame_decode(const uint8_t *src,
                           size_t size,
                           h3c_frame_t *frame,
                           size_t *frame_size)
{
  assert(src);
  assert(frame);
  assert(frame_size);

  *frame_size = 0;

  TRY_VARINT_DECODE_1(frame->type);

  uint64_t payload_size = 0;
  TRY_VARINT_DECODE_1(payload_size);

  switch (frame->type) {
    case H3C_FRAME_DATA:
      BUFFER_DECODE(frame->data.payload);
      break;
    case H3C_FRAME_HEADERS:
      BUFFER_DECODE(frame->headers.header_block);
      break;
    case H3C_FRAME_PRIORITY:;
      uint8_t byte = 0;
      TRY_UINT8_DECODE(byte);
      frame->priority.prioritized_element_type = byte >> 6;
      frame->priority.element_dependency_type = (byte >> 4) & 0x03;

      TRY_VARINT_DECODE_2(frame->priority.prioritized_element_id);
      TRY_VARINT_DECODE_2(frame->priority.element_dependency_id);

      TRY_UINT8_DECODE(frame->priority.weight);
      break;
    case H3C_FRAME_CANCEL_PUSH:
      TRY_VARINT_DECODE_2(frame->cancel_push.push_id);
      break;
    case H3C_FRAME_SETTINGS:
      frame->settings = h3c_settings_default;

      while (payload_size > 0) {
        uint64_t id = 0;
        TRY_VARINT_DECODE_2(id);

        switch (id) {
          case H3C_SETTINGS_MAX_HEADER_LIST_SIZE:
            TRY_SETTING_DECODE(H3C_SETTINGS_MAX_HEADER_LIST_SIZE,
                               frame->settings.max_header_list_size, uint64_t);
            break;
          case H3C_SETTINGS_NUM_PLACEHOLDERS:
            TRY_SETTING_DECODE(H3C_SETTINGS_NUM_PLACEHOLDERS,
                               frame->settings.num_placeholders, uint64_t);
            break;
          case H3C_SETTINGS_QPACK_MAX_TABLE_CAPACITY:
            TRY_SETTING_DECODE(H3C_SETTINGS_QPACK_MAX_TABLE_CAPACITY,
                               frame->settings.qpack_max_table_capacity,
                               uint32_t);
            break;
          case H3C_SETTINGS_QPACK_BLOCKED_STREAMS:
            TRY_SETTING_DECODE(H3C_SETTINGS_QPACK_BLOCKED_STREAMS,
                               frame->settings.qpack_blocked_streams, uint16_t);
            break;
          default:;
            // Unknown setting id => ignore its value
            uint64_t value = 0;
            TRY_VARINT_DECODE_2(value);
        }
      }
      break;
    case H3C_FRAME_PUSH_PROMISE:
      TRY_VARINT_DECODE_2(frame->push_promise.push_id);
      BUFFER_DECODE(frame->push_promise.header_block);
      break;
    case H3C_FRAME_GOAWAY:
      TRY_VARINT_DECODE_2(frame->goaway.stream_id);
      break;
    case H3C_FRAME_MAX_PUSH_ID:
      TRY_VARINT_DECODE_2(frame->max_push_id.push_id);
      break;
    case H3C_FRAME_DUPLICATE_PUSH:
      TRY_VARINT_DECODE_2(frame->duplicate_push.push_id);
      break;
  }

  if (payload_size > 0) {
    return H3C_ERROR_MALFORMED_FRAME;
  }

  return H3C_SUCCESS;
}
