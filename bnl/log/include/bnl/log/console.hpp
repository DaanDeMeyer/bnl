#pragma once

#include <bnl/log.hpp>
#include <bnl/log/export.hpp>

namespace bnl {
namespace log {

class BNL_LOG_EXPORT console : public log::api {
public:
  explicit console(bool log_source = true);

  void log(log::level level,
           const char *file,
           const char *function,
           int line,
           const char *format,
           const fmt::format_args &args) const final;

private:
  log::level level_ = log::level::trace;
  bool log_source = true;
};

}
}
