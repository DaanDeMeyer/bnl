#pragma once

#include <h3c/error.h>
#include <h3c/export.h>
#include <h3c/settings.h>

#include <stddef.h>
#include <stdint.h>

// https://quicwg.org/base-drafts/draft-ietf-quic-http.html#rfc.section.4

#ifdef __cplusplus
extern "C" {
#endif

typedef struct h3c_log_t h3c_log_t;

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
// Only valid for prioritized_element_type.
#define H3C_FRAME_PRIORITY_CURRENT 0x3U
// Only valid for element_dependency_type.
#define H3C_FRAME_PRIORITY_ROOT 0x3U

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

typedef h3c_settings_t h3c_frame_settings_t;

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

H3C_EXPORT size_t h3c_frame_encoded_size(const h3c_frame_t *frame);

H3C_EXPORT H3C_ERROR h3c_frame_encode(uint8_t *dest,
                                      size_t size,
                                      const h3c_frame_t *frame,
                                      size_t *encoded_size,
                                      h3c_log_t *log);

H3C_EXPORT H3C_ERROR h3c_frame_decode(const uint8_t *src,
                                      size_t size,
                                      h3c_frame_t *frame,
                                      size_t *encoded_size,
                                      h3c_log_t *log);

#ifdef __cplusplus
}
#endif
