#include <bnl/base/error.hpp>

namespace bnl {
namespace base {

class error_category_impl : public std::error_category {

public:
  const char *name() const noexcept override;

  std::string message(int condition) const noexcept override;
};

const char *error_category_impl::name() const noexcept
{
  return "bnl-base";
}

std::string error_category_impl::message(int condition) const noexcept
{
  switch (static_cast<error>(condition)) {
    case error::internal:
      return "internal";
    case error::not_implemented:
      return "not implemented";
    case error::invalid_argument:
      return "invalid argument";
    case error::incomplete:
      return "incomplete";
    case error::idle:
      return "idle";
    case error::unknown:
      return "unknown";
  }

  return "error not recognized";
}

const std::error_category &error_category() noexcept
{
  static error_category_impl instance;
  return instance;
}

std::error_code make_error_code(error error) noexcept
{
  return { static_cast<int>(error), error_category() };
}

} // namespace base
} // namespace bnl
