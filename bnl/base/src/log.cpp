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

}
}
