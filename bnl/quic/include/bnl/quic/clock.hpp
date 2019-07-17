#pragma once

#include <bnl/base/result.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <system_error>

namespace bnl {
namespace quic {

using duration = std::chrono::duration<uint64_t, std::micro>;
using milliseconds = std::chrono::duration<uint64_t, std::milli>;

using clock = std::function<base::result<duration>()>;

}
}
