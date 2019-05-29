#pragma once

#include <h3c/log.hpp>
#include <h3c/log/export.hpp>

#include <cstdio>

namespace h3c {
namespace log {
namespace impl {

class fprintf {
public:
  H3C_LOG_EXPORT void operator()(log::level level,
                                 const char *file,
                                 const char *function,
                                 int line,
                                 const char *format,
                                 const fmt::format_args &args);

private:
  log::level level_ = log::level::trace;
  FILE *out_ = nullptr;
  FILE *err_ = nullptr;
};

} // namespace impl
} // namespace log
} // namespace h3c
