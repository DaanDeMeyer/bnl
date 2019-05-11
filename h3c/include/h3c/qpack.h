#pragma once

#include <h3c/error.h>
#include <h3c/header.h>

// https://quicwg.org/base-drafts/draft-ietf-quic-qpack.html

// This QPACK implementation only uses the static table to keep the
// implementation and usage simple. Users should advertise
// `SETTINGS_QPACK_MAX_TABLE_CAPACITY` as zero when using this implementation.

#ifdef __cplusplus
extern "C" {
#endif

typedef struct h3c_log_t h3c_log_t;

H3C_EXPORT H3C_ERROR h3c_qpack_encode_prefix(uint8_t *dest,
                                             size_t size,
                                             size_t *encoded_size,
                                             h3c_log_t *log);

H3C_EXPORT H3C_ERROR h3c_qpack_encode(uint8_t *dest,
                                      size_t size,
                                      const h3c_header_t *header,
                                      size_t *encoded_size,
                                      h3c_log_t *log);

typedef struct {
  h3c_header_t huffman_decoded;
} h3c_qpack_decode_context_t;

H3C_EXPORT H3C_ERROR
h3c_qpack_decode_context_init(h3c_qpack_decode_context_t *context,
                              h3c_log_t *log);

H3C_EXPORT void
h3c_qpack_decode_context_destroy(h3c_qpack_decode_context_t *context);

H3C_EXPORT H3C_ERROR h3c_qpack_decode_prefix(const uint8_t *src,
                                             size_t size,
                                             size_t *encoded_size,
                                             h3c_log_t *log);

H3C_EXPORT H3C_ERROR h3c_qpack_decode(h3c_qpack_decode_context_t *context,
                                      const uint8_t *src,
                                      size_t size,
                                      h3c_header_t *header,
                                      size_t *encoded_size,
                                      h3c_log_t *log);

#ifdef __cplusplus
}
#endif
