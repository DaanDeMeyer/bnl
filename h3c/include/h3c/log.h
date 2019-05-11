#pragma once

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
  void (*log)(void *context,
                 H3C_LOG_LEVEL level,
                 const char *file,
                 const char *function,
                 int line,
                 const char *format,
                 ...);
  void *context;
} h3c_log_t;

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

#ifdef __cplusplus
}
#endif
