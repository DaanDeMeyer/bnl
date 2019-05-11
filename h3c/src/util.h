#pragma once

#include <h3c/error.h>
#include <h3c/log.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define H3C_ERROR(error)                                                       \
  switch ((error)) {                                                           \
    case H3C_SUCCESS:                                                          \
      break;                                                                   \
    default:                                                                   \
      H3C_LOG_ERROR("%s", h3c_strerror((error)));                              \
      break;                                                                   \
  }                                                                            \
                                                                               \
  return (error);                                                              \
  (void) 0
