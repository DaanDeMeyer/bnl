#pragma once

#include <h3c/error.h>
#include <h3c/log.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define H3C_LOG(level, format, ...)                                            \
  if (log && log->log) {                                                       \
    log->log(log->context, level, __FILE__, __func__, __LINE__, format,        \
             __VA_ARGS__);                                                     \
  }                                                                            \
  (void) 0

// clang-format off
#define H3C_LOG_TRACE(format, ...)   H3C_LOG(H3C_LOG_LEVEL_TRACE,   format, __VA_ARGS__)
#define H3C_LOG_DEBUG(format, ...)   H3C_LOG(H3C_LOG_LEVEL_DEBUG,   format, __VA_ARGS__)
#define H3C_LOG_INFO(format, ...)    H3C_LOG(H3C_LOG_LEVEL_INFO,    format, __VA_ARGS__)
#define H3C_LOG_WARNING(format, ...) H3C_LOG(H3C_LOG_LEVEL_WARNING, format, __VA_ARGS__)
#define H3C_LOG_ERROR(format, ...)   H3C_LOG(H3C_LOG_LEVEL_ERROR,   format, __VA_ARGS__)
// clang-format on

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
