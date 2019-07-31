#include <os/ip/address.hpp>

#include <arpa/inet.h>

namespace os {

result<ip::address>
make_address(base::string_view address)
{
  base::string string(address.data(), address.size());

  std::array<uint8_t, ipv4::address::size> ipv4 = {};
  int rv = inet_pton(AF_INET, string.c_str(), ipv4.data());
  if (rv == 1) {
    return ipv4::address({ ipv4.data(), ipv4.size() });
  }

  if (rv != 0) {
    THROW_SYSTEM(inet_pton, errno);
  }

  std::array<uint8_t, ipv6::address::size> ipv6 = {};
  rv = inet_pton(AF_INET6, string.c_str(), ipv6.data());
  if (rv != 1) {
    THROW_SYSTEM(inet_pton, errno);
  }

  return ipv6::address({ ipv6.data(), ipv6.size() });
}

}
