#pragma once

#include <bnl/base/export.hpp>
#include <bnl/result.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>

BNL_BASE_EXPORT std::ostream &
operator<<(std::ostream &os, const bnl::status_code_domain::string_ref &string);

namespace bnl {
namespace log {

enum class level {
  trace,
  debug,
  info,
  warning,
  error,
};

class BNL_BASE_EXPORT api {
public:
  api() = default;

  virtual ~api() noexcept;

  template<typename... Args>
  void operator()(log::level level,
                  const char *file,
                  const char *function,
                  int line,
                  const char *format,
                  const Args &... args) const
  {
    log(level, file, function, line, format, fmt::make_format_args(args...));
  }

protected:
  virtual void log(log::level level,
                   const char *file,
                   const char *function,
                   int line,
                   const char *format,
                   const fmt::format_args &args) const;
};

}

BNL_BASE_EXPORT extern log::api *logger;

}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#define BNL_LOG(level, format, ...)                                            \
  if (logger != nullptr) {                                                     \
    (logger)->operator()((level),                                              \
                         __FILE__,                                             \
                         static_cast<const char *>(__func__),                  \
                         __LINE__,                                             \
                         (format),                                             \
                         ##__VA_ARGS__);                                       \
  }                                                                            \
  (void) 0

#define BNL_LOG_T(format, ...)                                                 \
  BNL_LOG(bnl::log::level::trace, (format), ##__VA_ARGS__)

#define BNL_LOG_D(format, ...)                                                 \
  BNL_LOG(bnl::log::level::debug, (format), ##__VA_ARGS__)

#define BNL_LOG_I(format, ...)                                                 \
  BNL_LOG(bnl::log::level::info, (format), ##__VA_ARGS__)

#define BNL_LOG_W(format, ...)                                                 \
  BNL_LOG(bnl::log::level::warning, (format), ##__VA_ARGS__)

#define BNL_LOG_E(format, ...)                                                 \
  BNL_LOG(bnl::log::level::error, (format), ##__VA_ARGS__)

#pragma clang diagnostic pop
