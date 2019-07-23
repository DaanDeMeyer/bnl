#include <os/dns/client.hpp>
#include <os/error.hpp>

#include <netdb.h>

static ip::address
make_address(sockaddr *sockaddr)
{
  switch (sockaddr->sa_family) {
    case AF_INET: {
      sockaddr_in *ipv4 = reinterpret_cast<sockaddr_in *>(sockaddr);
      return ipv4::address(ipv4->sin_addr.s_addr);
    }

    case AF_INET6: {
      const sockaddr_in6 *ipv6 =
        reinterpret_cast<const sockaddr_in6 *>(sockaddr);

      const uint8_t *bytes =
        reinterpret_cast<const uint8_t *>(ipv6->sin6_addr.s6_addr);

      return ipv6::address({ bytes, ipv6::address::size });
    }
  }

  assert(false);
  return {};
}

namespace os {
namespace dns {

client::client(const log::api *logger)
  : logger_(logger)
{}

result<std::vector<ip::address>>
client::resolve(ip::host host)
{
  base::string name(host.name().data(), host.name().size());

  addrinfo hints = {};
  hints.ai_family = AF_UNSPEC;

  addrinfo *results = nullptr;

  int rv = getaddrinfo(name.c_str(), nullptr, &hints, &results);
  if (rv != 0) {
    THROW_SYSTEM(getaddrinfo, rv == EAI_SYSTEM ? errno : rv);
  }

  addrinfo *info = nullptr;

  std::vector<ip::address> addresses;

  for (info = results; info != nullptr; info = info->ai_next) {
    ip::address address = make_address(info->ai_addr);
    if (std::find(addresses.begin(), addresses.end(), address) ==
        addresses.end()) {
      addresses.push_back(address);
    }
  }

  return addresses;
}

}
}
