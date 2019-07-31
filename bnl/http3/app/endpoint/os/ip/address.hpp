#pragma once

#include <os/result.hpp>

#include <bnl/base/string_view.hpp>
#include <bnl/ip/address.hpp>

using namespace bnl;

namespace os {

result<ip::address>
make_address(base::string_view address);

}
