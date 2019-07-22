#pragma once

#include <bnl/ip/address.hpp>
#include <bnl/ip/port.hpp>

namespace bnl {
namespace ip {

class BNL_BASE_EXPORT endpoint {
public:
  endpoint() = default;
  endpoint(ip::address address, ip::port port) noexcept;

  endpoint(const endpoint &) = default;
  endpoint &operator=(const endpoint &) = default;

  const ip::address &address() const noexcept;
  ip::port port() const noexcept;

private:
  ip::address address_;
  ip::port port_ = 0;
};

}
}
