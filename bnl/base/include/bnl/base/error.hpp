#pragma once

#include <bnl/base/export.hpp>
#include <bnl/result.hpp>

namespace bnl {
namespace base {

enum class error { incomplete, idle, delegate, finished };

class domain;

using code = status_code<domain>;

class domain : public status_code_domain {
public:
  using value_type = error;

  constexpr domain() noexcept
    : status_code_domain(0xa5e52fd28d72a73e)
  {}

  domain(const domain &) = default;
  domain(domain &&) = default;
  domain &operator=(const domain &) = default;
  domain &operator=(domain &&) = default;
  ~domain() = default;

  static inline constexpr const domain &get();

  string_ref name() const noexcept final
  {
    return string_ref("bnl-base");
  }

  bool _do_failure(const status_code<void> &sc) const noexcept final
  {
    (void) sc;
    return true;
  }

  bool _do_equivalent(const status_code<void> &first,
                      const status_code<void> &second) const noexcept override
  {
    assert(first.domain() == *this);

    if (second.domain() == *this) {
      return static_cast<const code &>(first).value() ==
             static_cast<const code &>(second).value();
    }

    return false;
  }

  generic_code _generic_code(const status_code<void> &sc) const noexcept final
  {
    (void) sc;
    return errc::unknown;
  }

  string_ref _do_message(const status_code<void> &sc) const noexcept final
  {
    switch (static_cast<const code &>(sc).value()) {
      case error::incomplete:
        return string_ref("incomplete");
      case error::idle:
        return string_ref("idle");
      case error::delegate:
        return string_ref("delegate");
      case error::finished:
        return string_ref("finished");
    }

    return string_ref("unknown");
  }

#if defined(_CPPUNWIND) || defined(__EXCEPTIONS) || 0
  void _do_throw_exception(const status_code<void> &sc) const final
  {
    throw status_error<domain>(static_cast<const code &>(sc));
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
  return code(in_place, error);
}

}
}
