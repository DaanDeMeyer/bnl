#include <bnl/log.hpp>

namespace bnl {
namespace log {

api::~api() noexcept = default;

void api::operator()(const char *file,
                     const char *function,
                     int line,
                     std::error_code ec) const
{
  log(log::level::error, file, function, line, "{}",
      fmt::make_format_args(ec.message()));
}

void api::log(log::level level,
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

} // namespace log
} // namespace bnl
