#pragma once

#include <bnl/quic/error.hpp>
#include <bnl/result.hpp>

#include <cstdint>

namespace bnl {
namespace http3 {

enum class error {
  not_implemented,
  varint_overflow,
  malformed_header,
  stream_closed,
  invalid_handle
};

class domain;
using code = status_code<domain>;

class domain : public outcome::status_code_domain {
public:
  using value_type = error;

  constexpr domain() noexcept
    : status_code_domain(0x5c9e37a99f3fc4e4)
  {}

  domain(const domain &) = default;
  domain(domain &&) = default;
  domain &operator=(const domain &) = default;
  domain &operator=(domain &&) = default;
  ~domain() = default;

  static inline constexpr const domain &get();

  string_ref name() const noexcept final
  {
    return string_ref("bnl-http3");
  }

  bool _do_failure(const status_code<void> &sc) const noexcept final
  {
    (void) sc;
    return true;
  }

  bool _do_equivalent(const status_code<void> &first,
                      const status_code<void> &second) const noexcept final
  {
    if (second.domain() == *this) {
      return static_cast<const code &>(first).value() ==
             static_cast<const code &>(second).value();
    }

    return false;
  }

  outcome::generic_code _generic_code(const status_code<void> &sc) const
    noexcept final
  {
    (void) sc;
    return errc::unknown;
  }

  string_ref _do_message(const status_code<void> &sc) const noexcept final
  {
    switch (static_cast<const code &>(sc).value()) {
      case error::not_implemented:
        return string_ref("not implemented");
      case error::varint_overflow:
        return string_ref("varint overflow");
      case error::malformed_header:
        return string_ref("malformed header");
      case error::stream_closed:
        return string_ref("stream closed");
      case error::invalid_handle:
        return string_ref("invalid handle");
    }

    return string_ref("unknown");
  }

#if defined(_CPPUNWIND) || defined(__EXCEPTIONS) || 0
  void _do_throw_exception(const status_code<void> &sc) const final
  {

    throw outcome::status_error<domain>(static_cast<const code &>(sc));
  }
#endif
};

constexpr domain instance;

inline constexpr const domain &
domain::get()
{
  return instance;
}

inline code
make_status_code(error error)
{
  return code(outcome::in_place, error);
}

namespace connection {

enum class error : uint32_t {
  no_error = 0x0,
  internal = 0x03,
  wrong_stream = 0x0a,
  closed_critical_stream = 0x0f,
  missing_settings = 0x0012,
  unexpected_frame = 0x0013,
  malformed_frame = 0x0100,
  qpack_decompression_failed = 0x200
};

class domain;
using code = status_code<domain>;

class domain : public outcome::status_code_domain {
public:
  using value_type = error;

  constexpr domain() noexcept
    : status_code_domain(0x736db6f03aafc1ed)
  {}

  domain(const domain &) = default;
  domain(domain &&) = default;
  domain &operator=(const domain &) = default;
  domain &operator=(domain &&) = default;
  ~domain() = default;

  static inline constexpr const domain &get();

  string_ref name() const noexcept final
  {
    return string_ref("bnl-http3-protocol");
  }

  bool _do_failure(const status_code<void> &sc) const noexcept final
  {
    (void) sc;
    return true;
  }

  bool _do_equivalent(const status_code<void> &first,
                      const status_code<void> &second) const noexcept final
  {
    if (second.domain() == *this) {
      return static_cast<const code &>(first).value() ==
             static_cast<const code &>(second).value();
    }

    return false;
  }

  outcome::generic_code _generic_code(const status_code<void> &sc) const
    noexcept final
  {
    (void) sc;
    return errc::unknown;
  }

  string_ref _do_message(const status_code<void> &sc) const noexcept final
  {
    switch (static_cast<const code &>(sc).value()) {
      case error::no_error:
        return string_ref("no error");
      case error::internal:
        return string_ref("internal");
      case error::wrong_stream:
        return string_ref("wrong stream");
      case error::closed_critical_stream:
        return string_ref("closed critical stream");
      case error::missing_settings:
        return string_ref("missing settings");
      case error::unexpected_frame:
        return string_ref("unexpected frame");
      case error::malformed_frame:
        return string_ref("malformed frame");
      case error::qpack_decompression_failed:
        return string_ref("qpack decompression failed");
    }

    return string_ref("unknown");
  }

#if defined(_CPPUNWIND) || defined(__EXCEPTIONS) || 0
  void _do_throw_exception(const status_code<void> &sc) const final
  {
    throw outcome::status_error<domain>(static_cast<const code &>(sc));
  }
#endif
};

constexpr domain instance;

inline constexpr const domain &
domain::get()
{
  return instance;
}

inline code
make_status_code(error error)
{
  return code(outcome::in_place, error);
}

} // namespace connection

}
}
