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

#ifdef __cplusplus
}
#endif
