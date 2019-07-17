#include <bnl/quic/client/ngtcp2/error.hpp>

#include <ngtcp2/ngtcp2.h>

namespace bnl {
namespace quic {
namespace client {
namespace ngtcp2 {

class error_category_impl : public std::error_category {

public:
  const char *name() const noexcept override;

  std::string message(int condition) const noexcept override;
};

const char *
error_category_impl::name() const noexcept
{
  return "ngtcp2";
}

std::string
error_category_impl::message(int condition) const noexcept
{
  return ngtcp2_strerror(condition);
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

}
}
}
}
