#include <h3c/log/fprintf.h>

#include <assert.h>
#include <stdarg.h>

static const char *level_names[] = { "TRACE", "DEBUG", "INFO",
                                     "WARN",  "ERROR", "FATAL" };

static const char *level_colors[] = { "\x1b[94m", "\x1b[36m", "\x1b[32m",
                                      "\x1b[33m", "\x1b[31m", "\x1b[35m" };

void h3c_log_fprintf(void *context,
                     H3C_LOG_LEVEL level,
                     const char *file,
                     const char *function,
                     int line,
                     const char *format,
                     ...)
{
  h3c_log_fprintf_t *ctx = context;

  if (level < ctx->level) {
    return;
  }

  time_t global_time = time(NULL);
  struct tm *local_time = localtime(&global_time);

  char buffer[16];
  if (strftime(buffer, sizeof(buffer), "%H:%M:%S", local_time) == 0) {
    // TODO: Handle error
    assert(0);
    return;
  }

  FILE *output;

  switch (level) {
    case H3C_LOG_LEVEL_TRACE:
    case H3C_LOG_LEVEL_DEBUG:
    case H3C_LOG_LEVEL_INFO:
      output = ctx->out ? ctx->out : stdout;
      break;
    case H3C_LOG_LEVEL_WARNING:
    case H3C_LOG_LEVEL_ERROR:
      output = ctx->err ? ctx->err : stderr;
      break;
    default:
      // TODO: Handle error
      assert(0);
      return;
  }

  fprintf(output, "%s %s%-5s\x1b[0m \x1b[90m%s:%s:%d:\x1b[0m ", buffer,
          level_colors[level], level_names[level], file, function, line);

  va_list args;
  va_start(args, format);
  vfprintf(output, format, args);
  va_end(args);

  fprintf(output, "\n");

  fflush(output);
}
