#include <os/result.hpp>

class error_category_impl : public std::error_category {
public:
  const char *name() const noexcept override;

  std::string message(int code) const noexcept override;
};

const char *
error_category_impl::name() const noexcept
{
  return "bnl-http3";
}

std::string
error_category_impl::message(int code) const noexcept
{
  switch (static_cast<error>(code)) {
    case error::timeout:
      return "timeout";
    case error::resolve:
      return "resolve";
    case error::invalid_argument:
      return "invalid argument";
    case error::idle:
      return "idle";
    case error::finished:
      return "finished";
  }

  return "unknown";
}

const std::error_category &
error_category() noexcept
{
  static error_category_impl instance;
  return instance;
}

std::error_code
make_error_code(error error) noexcept
{
  return { static_cast<int>(error), error_category() };
}
