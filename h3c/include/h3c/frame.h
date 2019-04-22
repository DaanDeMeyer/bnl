#pragma once

#include <h3c/export.h>

#include <stddef.h>
#include <stdint.h>

// https://quicwg.org/base-drafts/draft-ietf-quic-http.html#rfc.section.4

#ifdef __cplusplus
extern "C" {
#endif

// Enums are always integers, we need unsigned types so we make do with typedefs
// and defines.

typedef uint64_t H3C_FRAME_TYPE;
#define H3C_FRAME_DATA 0x0U
#define H3C_FRAME_HEADERS 0x1U
#define H3C_FRAME_PRIORITY 0x2U
#define H3C_FRAME_CANCEL_PUSH 0x3U
#define H3C_FRAME_SETTINGS 0x4U
#define H3C_FRAME_PUSH_PROMISE 0x5U
#define H3C_FRAME_GOAWAY 0x7U
#define H3C_FRAME_MAX_PUSH_ID 0xdU
#define H3C_FRAME_DUPLICATE_PUSH 0xeU

typedef uint8_t H3C_FRAME_PRIORITY_TYPE;
#define H3C_FRAME_PRIORITY_REQUEST 0x0U
#define H3C_FRAME_PRIORITY_PUSH 0x1U
#define H3C_FRAME_PRIORITY_PLACEHOLDER 0x2U
#define H3C_FRAME_PRIORITY_CURRENT 0x3U // Only valid for prioritized_element_type.
#define H3C_FRAME_PRIORITY_ROOT 0x3U    // Only valid for element_dependency_type.

typedef struct {
  struct {
    uint64_t size;
  } payload;
} h3c_frame_data_t;

typedef struct {
  struct {
    uint64_t size;
  } header_block;
} h3c_frame_headers_t;

typedef struct {
  H3C_FRAME_PRIORITY_TYPE prioritized_element_type;
  H3C_FRAME_PRIORITY_TYPE element_dependency_type;
  uint64_t prioritized_element_id;
  uint64_t element_dependency_id;
  uint8_t weight;
} h3c_frame_priority_t;

typedef struct {
  uint64_t max_header_list_size;
  uint64_t num_placeholders;
  uint32_t qpack_max_table_capacity; // Max: 2^30 - 1
  uint16_t qpack_blocked_streams; // Max: 2^16 - 1
} h3c_frame_settings_t;

H3C_EXPORT extern const h3c_frame_settings_t h3c_frame_settings_default;

typedef struct {
  uint64_t push_id;
} h3c_frame_cancel_push_t;

typedef struct {
  uint64_t push_id;
  struct {
    uint64_t size;
  } header_block;
} h3c_frame_push_promise_t;

typedef struct {
  uint64_t stream_id;
} h3c_frame_goaway_t;

typedef struct {
  uint64_t push_id;
} h3c_frame_max_push_id_t;

typedef struct {
  uint64_t push_id;
} h3c_frame_duplicate_push_t;

typedef struct {
  H3C_FRAME_TYPE type;

  // We don't store the frame length since (for now) it's easier to calculate it
  // when needed. This also prevents it from getting stale.

  union {
    h3c_frame_data_t data;
    h3c_frame_headers_t headers;
    h3c_frame_priority_t priority;
    h3c_frame_settings_t settings;
    h3c_frame_cancel_push_t cancel_push;
    h3c_frame_push_promise_t push_promise;
    h3c_frame_goaway_t goaway;
    h3c_frame_max_push_id_t max_push_id;
    h3c_frame_duplicate_push_t duplicate_push;
  };
} h3c_frame_t;

typedef enum {
  H3C_FRAME_SERIALIZE_SUCCESS = 0,
  H3C_FRAME_SERIALIZE_BUF_TOO_SMALL = 1,
  H3C_FRAME_SERIALIZE_VARINT_OVERFLOW = 2
} H3C_FRAME_SERIALIZE_ERROR;

H3C_EXPORT H3C_FRAME_SERIALIZE_ERROR
h3c_frame_serialize(uint8_t *dest,
                    size_t size,
                    const h3c_frame_t *frame,
                    size_t *bytes_written);

typedef enum {
  H3C_FRAME_PARSE_SUCCESS = 0,
  H3C_FRAME_PARSE_INCOMPLETE = 1,
  H3C_FRAME_PARSE_MALFORMED = 2
} H3C_FRAME_PARSE_ERROR;

H3C_EXPORT H3C_FRAME_PARSE_ERROR h3c_frame_parse(const uint8_t *src,
                                                 size_t size,
                                                 h3c_frame_t *frame,
                                                 size_t *bytes_read);

#ifdef __cplusplus
}
#endif
