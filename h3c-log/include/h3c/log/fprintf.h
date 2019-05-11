#pragma once

#include <h3c/log/export.h>

#include <h3c/log.h>

#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  FILE *out;
  FILE *err;
  H3C_LOG_LEVEL level;
} h3c_log_fprintf_t;

H3C_LOG_EXPORT void h3c_log_fprintf(void *context,
                                    H3C_LOG_LEVEL level,
                                    const char *file,
                                    const char *function,
                                    int line,
                                    const char *format,
                                    ...);

#ifdef __cplusplus
}
#endif
