#pragma once

#include <h3c/export.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  H3C_SUCCESS = 0,

  // HTTP/3 connection errors

  H3C_ERROR_MALFORMED_FRAME = 0x0100,

  // HTTP/3 stream errors

  // h3c library errors

  // QUIC's error codes are limited to 16 bytes which leaves all numbers larger
  // than 16 bytes for library error codes.

  H3C_ERROR_INTERNAL = UINT16_MAX + 1,
  H3C_ERROR_BUFFER_TOO_SMALL,
  H3C_ERROR_VARINT_OVERFLOW,
  H3C_ERROR_SETTING_OVERFLOW,
  H3C_ERROR_INCOMPLETE
} H3C_ERROR;

H3C_EXPORT const char *h3c_strerror(H3C_ERROR error);

typedef enum {
  H3C_ERROR_TYPE_SUCCESS,
  H3C_ERROR_TYPE_CONNECTION,
  H3C_ERROR_TYPE_STREAM,
  H3C_ERROR_TYPE_LIBRARY
} H3C_ERROR_TYPE;

H3C_EXPORT H3C_ERROR_TYPE h3c_error_type(H3C_ERROR error);

#ifdef __cplusplus
}
#endif
