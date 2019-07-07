#include <bnl/log.hpp>

#include <bnl/error.hpp>

namespace bnl {
namespace log {

api::~api() noexcept = default;

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

void api::operator()(const char *file,
                     const char *function,
                     int line,
                     std::error_code ec) const
{
  if (ec.category() == core::error_category()) {
    switch (static_cast<core::error>(ec.value())) {
      case core::error::incomplete:
      case core::error::idle:
      case core::error::unknown:
        return;
      default:
        break;
    }
  }

  log(log::level::error, file, function, line, "{}",
      fmt::make_format_args(ec.message()));
}

} // namespace log
} // namespace bnl
