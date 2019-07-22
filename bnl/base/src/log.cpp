#include <bnl/log.hpp>

#include <bnl/base/error.hpp>

namespace bnl {
namespace log {

api::~api() noexcept = default;

void
api::log(log::level level,
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

void
api::operator()(const char *file,
                const char *function,
                int line,
                const status_code<void> &sc) const
{
  log::level level = log::level::error;
  fmt::string_view msg = { sc.message().data(), sc.message().size() };
  log(level, file, function, line, "{}", fmt::make_format_args(msg));
}

}
}
