#pragma once

#include <bnl/ip/address.hpp>
#include <bnl/ip/host.hpp>
#include <bnl/result.hpp>

#include <vector>

using namespace bnl;

namespace bnl {
namespace log {
class api;
}
}

namespace os {
namespace dns {

class client {
public:
  client(const log::api *logger);

  result<std::vector<ip::address>> resolve(const ip::host &host);

private:
  const log::api *logger_;
};

}
}
