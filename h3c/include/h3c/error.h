#pragma once

#include <h3c/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  H3C_SUCCESS = 0,
  H3C_BUF_TOO_SMALL = 1,
  H3C_VARINT_OVERFLOW = 2,
  H3C_SETTING_OVERFLOW = 3,
  H3C_FRAME_INCOMPLETE = 4,
  H3C_FRAME_MALFORMED = 5
} H3C_ERROR;

#ifdef __cplusplus
}
#endif
