#pragma once

#include <bnl/base/export.hpp>
#include <bnl/result.hpp>

#include <fmt/core.h>

namespace fmt {
template<>
struct formatter<bnl::status_code_domain::string_ref> {
  template<typename ParseContext>
  constexpr auto parse(ParseContext &ctx)
  {
    return ctx.begin();
  }

  template<typename FormatContext>
  auto format(const bnl::status_code_domain::string_ref &string,
              FormatContext &ctx)
  {
    return format_to(
      ctx.out(), "{}", fmt::string_view(string.data(), string.size()));
  }
};
}

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
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#define BNL_LOG(logger, level, format, ...)                                    \
  if (logger != nullptr) {                                                     \
    (logger)->operator()((level),                                              \
                         __FILE__,                                             \
                         static_cast<const char *>(__func__),                  \
                         __LINE__,                                             \
                         (format),                                             \
                         ##__VA_ARGS__);                                       \
  }                                                                            \
  (void) 0

#define BNL_LOG_TRACE(logger, format, ...)                                     \
  BNL_LOG(logger, bnl::log::level::trace, (format), ##__VA_ARGS__)

#define BNL_LOG_DEBUG(logger, format, ...)                                     \
  BNL_LOG(logger, bnl::log::level::debug, (format), ##__VA_ARGS__)

#define BNL_LOG_INFO(logger, format, ...)                                      \
  BNL_LOG(logger, bnl::log::level::info, (format), ##__VA_ARGS__)

#define BNL_LOG_WARNING(logger, format, ...)                                   \
  BNL_LOG(logger, bnl::log::level::warning, (format), ##__VA_ARGS__)

#define BNL_LOG_ERROR(logger, format, ...)                                     \
  BNL_LOG(logger, bnl::log::level::error, (format), ##__VA_ARGS__)

#pragma clang diagnostic pop
