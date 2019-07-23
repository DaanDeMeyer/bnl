#pragma once

#include <bnl/base/export.hpp>
#include <bnl/ip/address/ipv4.hpp>
#include <bnl/ip/address/ipv6.hpp>

#include <iosfwd>

namespace bnl {
namespace ip {

class BNL_BASE_EXPORT address {
public:
  address();
  address(ipv4::address address) noexcept; // NOLINT
  address(ipv6::address address) noexcept; // NOLINT
  address(base::buffer_view bytes) noexcept; // NOLINT

  address(const address &) = default;
  address &operator=(const address &) = default;

  enum class type { ipv4, ipv6 };

  operator type() const noexcept; // NOLINT

  base::buffer_view bytes() const noexcept;

  friend std::ostream &operator<<(std::ostream &os, const address &address);
  friend bool operator==(const address &lhs, const address &rhs);

private:
  type type_;

  union {
    ipv4::address ipv4_;
    ipv6::address ipv6_;
  };
};

}
}
