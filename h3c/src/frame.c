#include <h3c/frame.h>

#include <assert.h>
#include <string.h>

// Setting identifiers

#define SETTINGS_MAX_HEADER_LIST_SIZE 0x6U
#define SETTINGS_NUM_PLACEHOLDERS 0x9U

// Helper macros for `frame_parse`

// Macro to avoid having to manually check and update `src`, `size` and
// `bytes_read` values after each call to `varint_parse`.
#define TRY_VARINT_PARSE_1(value)                                              \
  {                                                                            \
    size_t rv = varint_parse(src, size, &(value));                             \
    if (rv == 0) {                                                             \
      return error;                                                            \
    }                                                                          \
                                                                               \
    src += rv;                                                                 \
    size -= rv;                                                                \
    *bytes_read += rv;                                                         \
  }                                                                            \
  (void) 0

// Additionally checks and updates `frame_length` after calling `varint_parse`.
#define TRY_VARINT_PARSE_2(value)                                              \
  {                                                                            \
    size_t rv = varint_parse(src, size, &(value));                             \
    if (rv == 0) {                                                             \
      return error;                                                            \
    }                                                                          \
                                                                               \
    if (rv > frame_length) {                                                   \
      return error;                                                            \
    }                                                                          \
                                                                               \
    src += rv;                                                                 \
    size -= rv;                                                                \
    *bytes_read += rv;                                                         \
    frame_length -= rv;                                                        \
  }                                                                            \
  (void) 0

// Casting `frame_length` to `size_t` in `BUFFER_PARSE` is safe since we don't
// continue parsing in `frame_parse` if `frame_length > size`, so when
// `BUFFER_PARSE` is first used, we're guaranteed that `frame_length <= size`
// (both are always subtracted by the same amount) and `frame_length` fits
// inside `size_t`.

#define BUFFER_PARSE(buffer)                                                   \
  (buffer).data = src;                                                         \
  (buffer).size = (size_t) frame_length;                                       \
  src += frame_length;                                                         \
  size -= (size_t) frame_length;                                               \
  *bytes_read += (size_t) frame_length;                                        \
  frame_length -= frame_length;                                                \
  (void) 0

FRAME_PARSE_ERROR frame_parse(const uint8_t *src, size_t size, frame_t *frame,
                              size_t *bytes_read)
{
  *bytes_read = 0;

  // Before parsing the frame's length, incomplete varints only indicate the
  // full frame is not yet available.
  FRAME_PARSE_ERROR error = FRAME_PARSE_INCOMPLETE;

  TRY_VARINT_PARSE_1(frame->type);

  varint_t frame_length = 0;
  TRY_VARINT_PARSE_1(frame_length);

  // We only continue parsing if the full frame payload is available in `src`.
  if (frame_length > size) {
    return error;
  }

  // From now on, if the frame's content size exceeds the frame's length or a
  // varint's size goes out of bounds of `src`, we're dealing with a malformed
  // frame.
  error = FRAME_PARSE_MALFORMED;

  switch (frame->type) {
  case HTTP3_DATA:
    BUFFER_PARSE(frame->data.payload);
    break;
  case HTTP3_HEADERS:
    BUFFER_PARSE(frame->headers.header_block);
    break;
  case HTTP3_PRIORITY:
    if (size == 0 || frame_length == 0) {
      return error;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    frame->priority.prioritized_element_type = *src >> 6;
    frame->priority.element_dependency_type = (*src >> 4) & 0x03;
#pragma GCC diagnostic pop

    src++;
    size--;
    frame_length--;

    TRY_VARINT_PARSE_2(frame->priority.prioritized_element_id);
    TRY_VARINT_PARSE_2(frame->priority.element_dependency_id);

    if (size == 0 || frame_length == 0) {
      return error;
    }

    frame->priority.weight = *src;
    src++;
    size--;
    frame_length--;
    break;
  case HTTP3_CANCEL_PUSH:
    TRY_VARINT_PARSE_2(frame->cancel_push.push_id);
    break;
  case HTTP3_SETTINGS:
    while (frame_length > 0) {
      varint_t id = 0;
      TRY_VARINT_PARSE_2(id);
      varint_t value = 0;
      TRY_VARINT_PARSE_2(value);

      switch (id) {
      case SETTINGS_MAX_HEADER_LIST_SIZE:
        frame->settings.max_header_list_size = value;
        break;
      case SETTINGS_NUM_PLACEHOLDERS:
        frame->settings.num_placeholders = value;
        break;
      }
    }
    break;
  case HTTP3_PUSH_PROMISE:
    TRY_VARINT_PARSE_2(frame->push_promise.push_id);
    BUFFER_PARSE(frame->push_promise.header_block);
    break;
  case HTTP3_GOAWAY:
    TRY_VARINT_PARSE_2(frame->goaway.stream_id);
    break;
  case HTTP3_MAX_PUSH_ID:
    TRY_VARINT_PARSE_2(frame->max_push_id.push_id);
    break;
  case HTTP3_DUPLICATE_PUSH:
    TRY_VARINT_PARSE_2(frame->duplicate_push.push_id);
    break;
  }

  if (frame_length > 0) {
    return error;
  }

  error = FRAME_PARSE_SUCCESS;

  return error;
}

// Helper macros for `frame_payload_size`

// Macro to avoid having to manually check the return value of `varint_size`
// each time we call it.
#define TRY_VARINT_SIZE(value)                                                 \
  {                                                                            \
    size_t rv = varint_size((value));                                          \
    if (rv == 0) {                                                             \
      return 0;                                                                \
    }                                                                          \
                                                                               \
    size += rv;                                                                \
  }                                                                            \
  (void) 0

static uint64_t frame_payload_size(const frame_t *frame)
{
  uint64_t size = 0;

  switch (frame->type) {
  case HTTP3_DATA:
    size += frame->data.payload.size;
    break;
  case HTTP3_HEADERS:
    size += frame->headers.header_block.size;
    break;
  case HTTP3_PRIORITY:
    size++; // PT size + DT size + Empty size = 1 byte. See
            // https://quicwg.org/base-drafts/draft-ietf-quic-http.html#frame-priority
    TRY_VARINT_SIZE(frame->priority.prioritized_element_id);
    TRY_VARINT_SIZE(frame->priority.element_dependency_id);
    size++; // Weight
    break;
  case HTTP3_CANCEL_PUSH:
    TRY_VARINT_SIZE(frame->cancel_push.push_id);
    break;
  case HTTP3_SETTINGS:
    TRY_VARINT_SIZE(SETTINGS_MAX_HEADER_LIST_SIZE);
    TRY_VARINT_SIZE(frame->settings.max_header_list_size);
    TRY_VARINT_SIZE(SETTINGS_NUM_PLACEHOLDERS);
    TRY_VARINT_SIZE(frame->settings.num_placeholders);
    break;
  case HTTP3_PUSH_PROMISE:
    TRY_VARINT_SIZE(frame->push_promise.push_id);
    size += frame->push_promise.header_block.size;
    break;
  case HTTP3_GOAWAY:
    TRY_VARINT_SIZE(frame->goaway.stream_id);
    break;
  case HTTP3_MAX_PUSH_ID:
    TRY_VARINT_SIZE(frame->max_push_id.push_id);
    break;
  case HTTP3_DUPLICATE_PUSH:
    TRY_VARINT_SIZE(frame->duplicate_push.push_id);
    break;
  }

  return size;
}

#define TRY_VARINT_SERIALIZE(value)                                            \
  {                                                                            \
    size_t rv = varint_serialize(dest, size, (value));                         \
    if (rv == 0) {                                                             \
      return error;                                                            \
    }                                                                          \
                                                                               \
    dest += rv;                                                                \
    size -= rv;                                                                \
    *bytes_written += rv;                                                      \
  }                                                                            \
  (void) 0

#define TRY_BUFFER_SERIALIZE(buffer)                                           \
  if ((buffer).size > size) {                                                  \
    return error;                                                              \
  }                                                                            \
                                                                               \
  memcpy(dest, (buffer).data, (buffer).size);                                  \
  dest += (buffer).size;                                                       \
  size -= (buffer).size;                                                       \
  *bytes_written += (buffer).size;                                             \
  (void) 0

FRAME_SERIALIZE_ERROR frame_serialize(uint8_t *dest, size_t size,
                                      const frame_t *frame,
                                      size_t *bytes_written)
{
  *bytes_written = 0;

  FRAME_SERIALIZE_ERROR error = FRAME_SERIALIZE_BUF_TOO_SMALL;

  TRY_VARINT_SERIALIZE(frame->type);

  varint_t frame_length = frame_payload_size(frame);
  if (frame_length == 0) {
    return FRAME_SERIALIZE_VARINT_OVERFLOW;
  }

  TRY_VARINT_SERIALIZE(frame_length);

  switch (frame->type) {
  case HTTP3_DATA:
    TRY_BUFFER_SERIALIZE(frame->data.payload);
    break;
  case HTTP3_HEADERS:
    TRY_BUFFER_SERIALIZE(frame->headers.header_block);
    break;
  case HTTP3_PRIORITY:
    if (size == 0) {
      return error;
    }

    *dest = (uint8_t)(*dest | frame->priority.prioritized_element_type << 6);
    *dest = (uint8_t)(*dest | frame->priority.element_dependency_type << 4);
    *dest &= 0xf0;
    dest++;
    size--;

    TRY_VARINT_SERIALIZE(frame->priority.prioritized_element_id);
    TRY_VARINT_SERIALIZE(frame->priority.element_dependency_id);

    if (size == 0) {
      return error;
    }

    *dest = frame->priority.weight;
    dest++;
    size--;
    break;
  case HTTP3_CANCEL_PUSH:
    TRY_VARINT_SERIALIZE(frame->cancel_push.push_id);
    break;
  case HTTP3_SETTINGS:
    TRY_VARINT_SERIALIZE(SETTINGS_MAX_HEADER_LIST_SIZE);
    TRY_VARINT_SERIALIZE(frame->settings.max_header_list_size);
    TRY_VARINT_SERIALIZE(SETTINGS_NUM_PLACEHOLDERS);
    TRY_VARINT_SERIALIZE(frame->settings.num_placeholders);
    break;
  case HTTP3_PUSH_PROMISE:
    TRY_VARINT_SERIALIZE(frame->push_promise.push_id);
    TRY_BUFFER_SERIALIZE(frame->push_promise.header_block);
    break;
  case HTTP3_GOAWAY:
    TRY_VARINT_SERIALIZE(frame->goaway.stream_id);
    break;
  case HTTP3_MAX_PUSH_ID:
    TRY_VARINT_SERIALIZE(frame->max_push_id.push_id);
    break;
  case HTTP3_DUPLICATE_PUSH:
    TRY_VARINT_SERIALIZE(frame->duplicate_push.push_id);
    break;
  }

  error = FRAME_SERIALIZE_SUCCESS;

  return error;
}
