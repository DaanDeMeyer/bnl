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
  (void)level;
  (void)file;
  (void)function;
  (void)line;
  (void)format;
  (void)args;
}

void
api::operator()(const char *file,
                const char *function,
                int line,
                std::error_code ec) const
{
  if (ec.category() == base::error_category()) {
    switch (static_cast<base::error>(ec.value())) {
      case base::error::incomplete:
      case base::error::idle:
      case base::error::unknown:
        return;
      default:
        break;
    }
  }

  log(log::level::error,
      file,
      function,
      line,
      "{}",
      fmt::make_format_args(ec.message()));
}

}
}
