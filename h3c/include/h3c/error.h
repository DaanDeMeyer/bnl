#pragma once

#include <h3c/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  H3C_SUCCESS = 0,
  H3C_ERROR_INTERNAL = 1,
  H3C_ERROR_BUF_TOO_SMALL = 2,
  H3C_ERROR_VARINT_OVERFLOW = 3,
  H3C_ERROR_SETTING_OVERFLOW = 4,
  H3C_ERROR_FRAME_INCOMPLETE = 5,
  H3C_ERROR_FRAME_MALFORMED = 6
} H3C_ERROR;

H3C_EXPORT const char *h3c_strerror(H3C_ERROR error);

#ifdef __cplusplus
}
#endif
