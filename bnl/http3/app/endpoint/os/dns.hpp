#pragma once

#include <os/result.hpp>

#include <bnl/ip/address.hpp>
#include <bnl/ip/host.hpp>

#include <vector>

using namespace bnl;

namespace os {
namespace dns {

result<std::vector<ip::address>>
resolve(const ip::host &host);

}
}
