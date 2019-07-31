#pragma once

#include <bnl/base/log.hpp>
#include <bnl/log/export.hpp>

namespace bnl {
namespace log {

class BNL_LOG_EXPORT console : public base::log::api {
public:
  explicit console(bool log_source = true);

  void log(base::log::level level,
           const char *file,
           const char *function,
           int line,
           const char *format,
           const fmt::format_args &args) const final;

private:
  base::log::level level_ = base::log::level::trace;
  bool log_source = true;
};

}
}
