#pragma once

#include <varint.h>

#include <stdint.h>

// https://quicwg.org/base-drafts/draft-ietf-quic-http.html#rfc.section.4

// An enum is always backed by a signed integer but we need a varint so we use
// defines and a typedef instead.

#ifdef __cplusplus
extern "C" {
#endif

typedef varint_t HTTP3_FRAME;
#define HTTP3_DATA 0x0U
#define HTTP3_HEADERS 0x1U
#define HTTP3_PRIORITY 0x2U
#define HTTP3_CANCEL_PUSH 0x3U
#define HTTP3_SETTINGS 0x4U
#define HTTP3_PUSH_PROMISE 0x5U
#define HTTP3_GOAWAY 0x7U
#define HTTP3_MAX_PUSH_ID 0xDU
#define HTTP3_DUPLICATE_PUSH 0xEU

typedef struct {
  HTTP3_FRAME type;

  // We don't store the frame length since (for now) it's easier to calculate it
  // when needed. This also prevents it from getting stale.

  union {
    struct {
      struct {
        const uint8_t *data;
        size_t size;
      } payload;
    } data;

    struct {
      struct {
        const uint8_t *data;
        size_t size;
      } header_block;
    } headers;

    struct {
      uint8_t prioritized_element_type;
      uint8_t element_dependency_type;
      varint_t prioritized_element_id;
      varint_t element_dependency_id;
      uint8_t weight;
    } priority;

    struct {
      varint_t push_id;
    } cancel_push;

    struct {
      varint_t max_header_list_size;
      varint_t num_placeholders;
    } settings;

    struct {
      varint_t push_id;
      struct {
        const uint8_t *data;
        size_t size;
      } header_block;
    } push_promise;

    struct {
      varint_t stream_id;
    } goaway;

    struct {
      varint_t push_id;
    } max_push_id;

    struct {
      varint_t push_id;
    } duplicate_push;
  };
} frame_t;

typedef enum {
  FRAME_PARSE_SUCCESS = 0,
  FRAME_PARSE_INCOMPLETE = 1,
  FRAME_PARSE_MALFORMED = 2
} FRAME_PARSE_ERROR;

FRAME_PARSE_ERROR frame_parse(const uint8_t *src, size_t size, frame_t *frame,
                              size_t *bytes_read);

typedef enum {
  FRAME_SERIALIZE_SUCCESS = 0,
  FRAME_SERIALIZE_BUF_TOO_SMALL = 1,
  FRAME_SERIALIZE_VARINT_OVERFLOW = 2
} FRAME_SERIALIZE_ERROR;

FRAME_SERIALIZE_ERROR frame_serialize(uint8_t *dest, size_t size,
                                      const frame_t *frame,
                                      size_t *bytes_written);

#ifdef __cplusplus
}
#endif
