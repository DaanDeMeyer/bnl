#include <h3c/frame.h>

#include <util.h>

#include <assert.h>

#define TRY_VARINT_SIZE(value)                                                 \
  {                                                                            \
    size_t varint_size = 0;                                                    \
    H3C_ERROR error = h3c_varint_encode(NULL, 0, (value), &varint_size, log);  \
    if (error) {                                                               \
      return error;                                                            \
    }                                                                          \
                                                                               \
    *size += varint_size;                                                      \
  }                                                                            \
  (void) 0

#define TRY_SETTING_SIZE(id, value)                                            \
  if ((value) > id##_MAX) {                                                    \
    H3C_ERROR(H3C_ERROR_SETTING_OVERFLOW);                                     \
  }                                                                            \
                                                                               \
  TRY_VARINT_SIZE((id));                                                       \
  TRY_VARINT_SIZE((value));                                                    \
  (void) 0

static H3C_ERROR
frame_payload_size(const h3c_frame_t *frame, uint64_t *size, h3c_log_t *log)
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
      TRY_SETTING_SIZE(H3C_SETTINGS_MAX_HEADER_LIST_SIZE,
                       frame->settings.max_header_list_size);
      TRY_SETTING_SIZE(H3C_SETTINGS_NUM_PLACEHOLDERS,
                       frame->settings.num_placeholders);
      TRY_SETTING_SIZE(H3C_SETTINGS_QPACK_MAX_TABLE_CAPACITY,
                       frame->settings.qpack_max_table_capacity);
      TRY_SETTING_SIZE(H3C_SETTINGS_QPACK_BLOCKED_STREAMS,
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
    H3C_ERROR(H3C_ERROR_VARINT_OVERFLOW);
  }

  return H3C_SUCCESS;
}

#define TRY_VARINT_ENCODE(value)                                               \
  {                                                                            \
    size_t varint_size = 0;                                                    \
    H3C_ERROR error = h3c_varint_encode(dest, size, (value), &varint_size,     \
                                        log);                                  \
    if (error) {                                                               \
      return error;                                                            \
    }                                                                          \
                                                                               \
    if (dest) {                                                                \
      dest += varint_size;                                                     \
      size -= varint_size;                                                     \
    }                                                                          \
                                                                               \
    *encoded_size += varint_size;                                              \
  }                                                                            \
  (void) 0

#define TRY_UINT8_ENCODE(value)                                                \
  if (dest) {                                                                  \
    if (size == 0) {                                                           \
      H3C_ERROR(H3C_ERROR_BUFFER_TOO_SMALL);                                   \
    }                                                                          \
                                                                               \
    *dest = (value);                                                           \
    dest++;                                                                    \
    size--;                                                                    \
  }                                                                            \
                                                                               \
  (*encoded_size)++;                                                           \
  (void) 0

#define TRY_SETTING_ENCODE(id, value)                                          \
  TRY_VARINT_ENCODE((id));                                                     \
  TRY_VARINT_ENCODE((value));                                                  \
  (void) 0

H3C_ERROR h3c_frame_encode(uint8_t *dest,
                           size_t size,
                           const h3c_frame_t *frame,
                           size_t *encoded_size,
                           h3c_log_t *log)
{
  assert(frame);
  assert(encoded_size);

  *encoded_size = 0;

  uint64_t payload_size = 0;
  H3C_ERROR error = frame_payload_size(frame, &payload_size, log);
  if (error) {
    return error;
  }

  TRY_VARINT_ENCODE(frame->type);
  TRY_VARINT_ENCODE(payload_size);

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
      TRY_UINT8_ENCODE(byte);

      TRY_VARINT_ENCODE(frame->priority.prioritized_element_id);
      TRY_VARINT_ENCODE(frame->priority.element_dependency_id);

      TRY_UINT8_ENCODE(frame->priority.weight);
      break;
    case H3C_FRAME_CANCEL_PUSH:
      TRY_VARINT_ENCODE(frame->cancel_push.push_id);
      break;
    case H3C_FRAME_SETTINGS:
      TRY_SETTING_ENCODE(H3C_SETTINGS_MAX_HEADER_LIST_SIZE,
                         frame->settings.max_header_list_size);
      TRY_SETTING_ENCODE(H3C_SETTINGS_NUM_PLACEHOLDERS,
                         frame->settings.num_placeholders);
      TRY_SETTING_ENCODE(H3C_SETTINGS_QPACK_MAX_TABLE_CAPACITY,
                         frame->settings.qpack_max_table_capacity);
      TRY_SETTING_ENCODE(H3C_SETTINGS_QPACK_BLOCKED_STREAMS,
                         frame->settings.qpack_blocked_streams);
      break;
    case H3C_FRAME_PUSH_PROMISE:
      TRY_VARINT_ENCODE(frame->push_promise.push_id);
      break;
    case H3C_FRAME_GOAWAY:
      TRY_VARINT_ENCODE(frame->goaway.stream_id);
      break;
    case H3C_FRAME_MAX_PUSH_ID:
      TRY_VARINT_ENCODE(frame->max_push_id.push_id);
      break;
    case H3C_FRAME_DUPLICATE_PUSH:
      TRY_VARINT_ENCODE(frame->duplicate_push.push_id);
      break;
  }

  return H3C_SUCCESS;
}
