#pragma once

#include <h3c/error.h>

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  H3C_LOG_LEVEL_TRACE,
  H3C_LOG_LEVEL_DEBUG,
  H3C_LOG_LEVEL_INFO,
  H3C_LOG_LEVEL_WARNING,
  H3C_LOG_LEVEL_ERROR
} H3C_LOG_LEVEL;

typedef struct h3c_log_t {
  void (*write)(void *context,
                H3C_LOG_LEVEL level,
                const char *file,
                const char *function,
                int line,
                const char *format,
                ...);
  void *context;
} h3c_log_t;

#define H3C_LOG(log, level, format, ...)                                       \
  if ((log) && (log)->write) {                                                 \
    (log)->write((log)->context, (level), __FILE__, __func__, __LINE__,        \
                 (format), __VA_ARGS__);                                       \
  }                                                                            \
  (void) 0

// clang-format off
#define H3C_LOG_TRACE(log, format, ...)   H3C_LOG(log, H3C_LOG_LEVEL_TRACE,   (format), __VA_ARGS__)
#define H3C_LOG_DEBUG(log, format, ...)   H3C_LOG(log, H3C_LOG_LEVEL_DEBUG,   (format), __VA_ARGS__)
#define H3C_LOG_INFO(log, format, ...)    H3C_LOG(log, H3C_LOG_LEVEL_INFO,    (format), __VA_ARGS__)
#define H3C_LOG_WARNING(log, format, ...) H3C_LOG(log, H3C_LOG_LEVEL_WARNING, (format), __VA_ARGS__)
#define H3C_LOG_ERROR(log, format, ...)   H3C_LOG(log, H3C_LOG_LEVEL_ERROR,   (format), __VA_ARGS__)
// clang-format on

#define H3C_THROW(log, error)                                                  \
  switch ((error)) {                                                           \
    case H3C_SUCCESS:                                                          \
      break;                                                                   \
    case H3C_ERROR_INTERNAL:                                                   \
      assert(0);                                                               \
    default:                                                                   \
      H3C_LOG_ERROR(log, "%s", h3c_strerror((error)));                         \
      break;                                                                   \
  }                                                                            \
                                                                               \
  return (error);                                                              \
  (void) 0

#ifdef __cplusplus
}
#endif
