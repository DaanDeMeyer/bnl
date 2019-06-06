#include <h3c/log/fprintf.hpp>

#include <fmt/chrono.h>
#include <fmt/color.h>

#include <array>

static constexpr std::array<const char *, 6> level_names = { "TRACE", "DEBUG",
                                                             "INFO",  "WARN",
                                                             "ERROR", "FATAL" };

static constexpr std::array<fmt::color, 6> level_colors = {
  fmt::color::light_blue, fmt::color::cyan, fmt::color::green,
  fmt::color::yellow,     fmt::color::red,  fmt::color::magenta
};

namespace h3c {
namespace log {
namespace impl {

void fprintf::operator()(log::level level,
                         const char *file,
                         const char *function,
                         int line,
                         const char *format,
                         const fmt::format_args &args)
{
  (void) function;

  if (level < level_) {
    return;
  }

  FILE *output = nullptr;

  switch (level) {
    case log::level::warning:
    case log::level::error:
      output = err_ != nullptr ? err_ : stderr;
      break;
    default:
      output = out_ != nullptr ? out_ : stdout;
  }

  fmt::text_style color;
  std::time_t time = std::time(nullptr);
  color = fmt::fg(fmt::color::light_gray);
  fmt::print(output, color, "{:%H:%M:%S} ", fmt::localtime(time));

  color = fmt::fg(level_colors[static_cast<size_t>(level)]);
  const char *level_name = level_names[static_cast<size_t>(level)];
  fmt::print(output, color, "{} ", level_name);

  color = fmt::fg(fmt::color::dark_gray);
  fmt::print(output, color, "{}:{}: ", file, line);

  color = fmt::fg(fmt::color::white);
  fmt::vprint(output, color, format, args);

  fmt::print("\n");

  fflush(output);
}

} // namespace impl
} // namespace log
} // namespace h3c
