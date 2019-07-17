#include <bnl/http3/error.hpp>

namespace bnl {
namespace http3 {

class error_category_impl : public std::error_category {

public:
  const char *name() const noexcept override;

  std::string message(int condition) const noexcept override;
};

const char *
error_category_impl::name() const noexcept
{
  return "bnl-http3";
}

std::string
error_category_impl::message(int condition) const noexcept
{
  switch (static_cast<error>(condition)) {
    case error::internal_error:
      return "internal error";
    case error::wrong_stream:
      return "wrong stream";
    case error::closed_critical_stream:
      return "closed critical stream";
    case error::missing_settings:
      return "missing settings";
    case error::unexpected_frame:
      return "unexpected frame";
    case error::malformed_frame:
      return "malformed frame";
    case error::qpack_decompression_failed:
      return "qpack decompression failed";

    case error::varint_overflow:
      return "varint overflow";
    case error::malformed_header:
      return "malformed header";
    case error::stream_closed:
      return "stream closed";
    case error::stream_exists:
      return "stream exists";
    case error::invalid_handle:
      return "invalid handle";
  }

  return "error not recognized";
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
