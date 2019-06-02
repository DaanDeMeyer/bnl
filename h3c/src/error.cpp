#include <h3c/error.hpp>

namespace h3c {

class error_category_impl : public std::error_category {

public:
  const char *name() const noexcept override;

  std::string message(int condition) const noexcept override;
};

const char *error_category_impl::name() const noexcept
{
  return "h3c";
}

std::string error_category_impl::message(int condition) const noexcept
{
  switch (static_cast<error>(condition)) {
      // HTTP/3 connection errors

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

      // h3c library errors

    case error::not_implemented:
      return "not implemented";
    case error::out_of_memory:
      return "out of memory";
    case error::buffer_too_small:
      return "buffer too small";
    case error::incomplete:
      return "incomplete";
    case error::varint_overflow:
      return "varint overflow";
    case error::setting_overflow:
      return "setting overflow";
    case error::concurrent_streams_limit_exceeded:
      return "concurrent streams limit exceeded";
    case error::malformed_header:
      return "malformed header";
    case error::stream_closed:
      return "stream closed";
    case error::unknown_frame_type:
      return "unknown frame type";
  }

  return "unknown error";
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

} // namespace h3c
