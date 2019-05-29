#pragma once

#include <h3c/export.hpp>

#include <fmt/core.h>

#include <functional>

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

class logger {
public:
  using api = void(log::level level,
                   const char *file,
                   const char *function,
                   int line,
                   const char *format,
                   const fmt::format_args &args);

  H3C_EXPORT explicit logger(std::function<api> impl = {})
      : impl_(std::move(impl))
  {}

  template <typename... Args>
  void log(log::level level,
           const char *file,
           const char *function,
           int line,
           const char *format,
           const Args &... args) const
  {
    if (!impl_) {
      return;
    }

    impl_(level, file, function, line, format, fmt::make_format_args(args...));
  }

private:
  std::function<api> impl_;
};

} // namespace h3c

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#define H3C_LOG(logger, level, format, ...)                                    \
  if (logger != nullptr) {                                                                \
    (logger)->log((level), __FILE__, static_cast<const char *>(__func__),      \
                  __LINE__, (format), ##__VA_ARGS__);                          \
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
