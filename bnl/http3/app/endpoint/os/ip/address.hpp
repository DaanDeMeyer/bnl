#pragma once

#include <bnl/base/string_view.hpp>
#include <bnl/ip/address.hpp>
#include <bnl/result.hpp>

using namespace bnl;

namespace os {

result<ip::address>
make_address(base::string_view address);

}
