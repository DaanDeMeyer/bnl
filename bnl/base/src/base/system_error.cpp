#include <bnl/base/system_error.hpp>

namespace bnl {

namespace http3 {

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
    case error::no_error:
      return "no error";
    case error::internal:
      return "internal";
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
    case error::not_implemented:
      return "not implemented";
    case error::varint_overflow:
      return "varint overflow";
    case error::malformed_header:
      return "malformed header";
    case error::stream_closed:
      return "stream closed";
    case error::invalid_handle:
      return "invalid handle";
    case error::incomplete:
      return "incomplete";
    case error::idle:
      return "idle";
    case error::delegate:
      return "delegate";
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

}

namespace quic {

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
    case error::no_error:
      return "no error";
    case error::internal:
      return "internal";
    case error::not_implemented:
      return "not implemented";
    case error::incomplete:
      return "incomplete";
    case error::idle:
      return "idle";
    case error::delegate:
      return "delegate";
    case error::finished:
      return "finished";
    case error::stream_id_blocked:
      return "stream id blocked";
    case error::stream_data_blocked:
      return "stream data blocked";
    case error::stream_not_found:
      return "stream not found";
    case error::handshake:
      return "handshake";
    case error::crypto:
      return "crypto";
    case error::ngtcp2:
      return "ngtcp2";
    case error::path_validation:
      return "path validation";
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

}

}
