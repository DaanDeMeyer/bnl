#include <bnl/quic/error.hpp>

namespace bnl {
namespace quic {

class error_category_impl : public std::error_category {

public:
  const char *name() const noexcept override;

  std::string message(int condition) const noexcept override;
};

const char *
error_category_impl::name() const noexcept
{
  return "bnl-quic";
}

std::string
error_category_impl::message(int condition) const noexcept
{
  switch (static_cast<error>(condition)) {
    case error::handshake:
      return "handshake";
    case error::crypto:
      return "crypto";
    case error::path_validation:
      return "path validation";
  }

  return "unknown error";
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

} // namespace quic
} // namespace bnl
