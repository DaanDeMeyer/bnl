#pragma once

#include <h3c/error.hpp>
#include <h3c/export.hpp>

#include <fmt/core.h>

namespace h3c {

namespace log {

enum class level {
  trace,
  debug,
  info,
  warning,
  error,
};

} // namespace log

class H3C_EXPORT logger {
public:
  template <typename... Args>
  void operator()(log::level level,
                  const char *file,
                  const char *function,
                  int line,
                  const char *format,
                  const Args &... args) const
  {
    log(level, file, function, line, format, fmt::make_format_args(args...));
  }

  void operator()(const char *file,
                  const char *function,
                  int line,
                  std::error_code ec) const
  {
    log::level level = log::level::error;

    if (ec.category() == h3c::error_category()) {
      switch (static_cast<h3c::error>(ec.value())) {
        case error::idle:
        case error::unknown:
        case error::incomplete:
          level = log::level::trace;
        default:
          break;
      }
    }

    log(level, file, function, line, "{}", fmt::make_format_args(ec.message()));
  }

protected:
  virtual void log(log::level level,
                   const char *file,
                   const char *function,
                   int line,
                   const char *format,
                   const fmt::format_args &args) const
  {
    (void) level;
    (void) file;
    (void) function;
    (void) line;
    (void) format;
    (void) args;
  }
};

} // namespace h3c

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#define H3C_LOG(logger, level, format, ...)                                    \
  if (logger != nullptr) {                                                     \
    (logger)->operator()((level), __FILE__,                                    \
                         static_cast<const char *>(__func__), __LINE__,        \
                         (format), ##__VA_ARGS__);                             \
  }                                                                            \
  (void) 0

#define H3C_LOG_TRACE(logger, format, ...)                                     \
  H3C_LOG(logger, h3c::log::level::trace, (format), ##__VA_ARGS__)

#define H3C_LOG_DEBUG(logger, format, ...)                                     \
  H3C_LOG(logger, h3c::log::level::debug, (format), ##__VA_ARGS__)

#define H3C_LOG_INFO(logger, format, ...)                                      \
  H3C_LOG(logger, h3c::log::level::info, (format), ##__VA_ARGS__)

#define H3C_LOG_WARNING(logger, format, ...)                                   \
  H3C_LOG(logger, h3c::log::level::warning, (format), ##__VA_ARGS__)

#define H3C_LOG_ERROR(logger, format, ...)                                     \
  H3C_LOG(logger, h3c::log::level::error, (format), ##__VA_ARGS__)

#pragma clang diagnostic pop
#pragma GCC diagnostic pop
