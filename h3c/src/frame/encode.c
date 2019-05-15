#include <h3c/frame.h>

#include <assert.h>

#define TRY_VARINT_SIZE(value)                                                 \
  {                                                                            \
    size_t varint_encoded_size = h3c_varint_encoded_size((value));             \
    /* Use the maximum varint size if the varint overflows. We return an       \
     * overflow error later when the frame is actually encoded. */             \
    varint_encoded_size = varint_encoded_size != 0 ? varint_encoded_size       \
                                                   : H3C_VARINT_UINT64_SIZE;   \
    payload_size += varint_encoded_size;                                       \
  }                                                                            \
  (void) 0

#define TRY_SETTING_SIZE(id, value)                                            \
  TRY_VARINT_SIZE((id));                                                       \
  TRY_VARINT_SIZE((value));                                                    \
  (void) 0

static uint64_t frame_payload_size(const h3c_frame_t *frame)
{
  uint64_t payload_size = 0;

  switch (frame->type) {
    case H3C_FRAME_DATA:
      payload_size += frame->data.payload.size;
      break;
    case H3C_FRAME_HEADERS:
      payload_size += frame->headers.header_block.size;
      break;
    case H3C_FRAME_PRIORITY:
      payload_size++; // PT size + DT size + Empty size = 1 byte. See
                      // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-priority
      TRY_VARINT_SIZE(frame->priority.prioritized_element_id);
      TRY_VARINT_SIZE(frame->priority.element_dependency_id);
      payload_size++; // Weight
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
      payload_size += frame->push_promise.header_block.size;
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

  return payload_size;
}

#define TRY_VARINT_ENCODE(value)                                               \
  {                                                                            \
    size_t varint_encoded_size = 0;                                            \
    H3C_ERROR error = h3c_varint_encode(dest, size, (value),                   \
                                        &varint_encoded_size, log);            \
    if (error) {                                                               \
      return error;                                                            \
    }                                                                          \
                                                                               \
    dest += varint_encoded_size;                                               \
    size -= varint_encoded_size;                                               \
    *encoded_size += varint_encoded_size;                                      \
  }                                                                            \
  (void) 0

#define TRY_UINT8_ENCODE(value)                                                \
  if (size == 0) {                                                             \
    H3C_THROW(H3C_ERROR_BUFFER_TOO_SMALL, log);                                \
  }                                                                            \
                                                                               \
  *dest = (value);                                                             \
  dest++;                                                                      \
  size--;                                                                      \
                                                                               \
  (*encoded_size)++;                                                           \
  (void) 0

#define TRY_SETTING_ENCODE(id, value)                                          \
  if ((value) > id##_MAX) {                                                    \
    H3C_LOG_ERROR(log, "Value of %s (%lu) exceeds maximum (%lu)", #id, value,  \
                  id##_MAX);                                                   \
    H3C_THROW(H3C_ERROR_SETTING_OVERFLOW, log);                                \
  }                                                                            \
                                                                               \
  TRY_VARINT_ENCODE((id));                                                     \
  TRY_VARINT_ENCODE((value));                                                  \
  (void) 0

size_t h3c_frame_encoded_size(const h3c_frame_t *frame)
{
  assert(frame);

  uint64_t payload_size = frame_payload_size(frame);
  size_t payload_encoded_size = 0;

  // `payload_size` includes the size of the DATA frame payload and
  // HEADERS/PUSH_PROMISE header block but these are not encoded in
  // `h3c_frame_encode` (this is left to the user) so we calculate the encoded
  // payload size of HEADERS, DATA and PUSH_PROMISE frames manually and delegate
  // the rest to `frame_payload_size`.
  switch (frame->type) {
    case H3C_FRAME_HEADERS:
    case H3C_FRAME_DATA:
      break;
    case H3C_FRAME_PUSH_PROMISE:
      payload_encoded_size = h3c_varint_encoded_size(
          frame->push_promise.push_id);
      break;
    default:
      // HEADERS, DATA and PUSH_PROMISE frames are the only frames that might
      // have a payload size exceeding a `size_t` value and those are handled
      // separately so the cast is safe.
      payload_encoded_size = (size_t) payload_size;
      break;
  }

  size_t encoded_size = 0;

  encoded_size += h3c_varint_encoded_size(frame->type);
  encoded_size += h3c_varint_encoded_size(payload_size);
  encoded_size += payload_encoded_size;

  return encoded_size;
}

H3C_ERROR h3c_frame_encode(uint8_t *dest,
                           size_t size,
                           const h3c_frame_t *frame,
                           size_t *encoded_size,
                           h3c_log_t *log)
{
  assert(dest);
  assert(frame);
  assert(encoded_size);

  *encoded_size = 0;

  uint64_t payload_size = frame_payload_size(frame);

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
