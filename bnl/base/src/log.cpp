#include <bnl/log.hpp>

#include <bnl/base/error.hpp>

#include <ostream>

std::ostream &
operator<<(std::ostream &os, const bnl::status_code_domain::string_ref &string)
{
  return os.write(string.data(), string.size());
}

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
