#pragma once

#include <bnl/base/export.hpp>
#include <bnl/result.hpp>

#include <cstdint>
#include <mutex>

namespace bnl {
namespace quic {

enum class error {
  not_implemented,
  handshake,
  crypto,
  path_validation,
  finished,
  stream_not_found
};

class domain;
using code = status_code<domain>;

class domain : public outcome::status_code_domain {
public:
  using value_type = error;

  constexpr domain() noexcept
    : status_code_domain(0x7b109e28661bc284)
  {}

  domain(const domain &) = default;
  domain(domain &&) = default;
  domain &operator=(const domain &) = default;
  domain &operator=(domain &&) = default;
  ~domain() = default;

  static inline constexpr const domain &get();

  string_ref name() const noexcept final
  {
    return string_ref("bnl-quic");
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
      case error::handshake:
        return string_ref("handshake");
      case error::crypto:
        return string_ref("crypto");
      case error::path_validation:
        return string_ref("path validation");
      case error::finished:
        return string_ref("finished");
      case error::stream_not_found:
        return string_ref("stream not found");
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

enum class error : uint64_t { no_error, internal };

class domain;
using code = status_code<domain>;

class domain : public outcome::status_code_domain {
public:
  using value_type = error;

  constexpr domain() noexcept
    : status_code_domain(0x5409718570396119)
  {}

  domain(const domain &) = default;
  domain(domain &&) = default;
  domain &operator=(const domain &) = default;
  domain &operator=(domain &&) = default;
  ~domain() = default;

  static inline constexpr const domain &get();

  string_ref name() const noexcept final
  {
    return string_ref("bnl-quic-protocol");
  }

  bool _do_failure(const status_code<void> &sc) const noexcept final
  {
    return static_cast<const code &>(sc).value() != error::no_error;
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

namespace application {

struct error {
  enum type { rst_stream, stop_sending, connection_close };

  const type type;
  const uint64_t id;
  const uint64_t value;
};

} // namespace application

}
}
