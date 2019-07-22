#pragma once

#include <bnl/result.hpp>

#include <chrono>
#include <cstdint>
#include <functional>

namespace bnl {
namespace quic {

using duration = std::chrono::duration<uint64_t, std::micro>;
using milliseconds = std::chrono::duration<uint64_t, std::milli>;

using clock = std::function<result<duration>()>;

}
}
