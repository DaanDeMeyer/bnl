#pragma once

#include <bnl/base/export.hpp>
#include <bnl/ip/address/ipv4.hpp>
#include <bnl/ip/address/ipv6.hpp>

namespace bnl {
namespace ip {

class BNL_BASE_EXPORT address
{
public:
  address();
  address(ipv4::address address) noexcept; // NOLINT
  address(ipv6::address address) noexcept; // NOLINT

  enum class type
  {
    ipv4,
    ipv6
  };

  operator type() const noexcept; // NOLINT

  base::buffer_view bytes() const noexcept;

private:
  const type type_;

public:
  union
  {
    const ipv4::address ipv4;
    const ipv6::address ipv6;
  };
};

} // namespace ip
} // namespace bnl
