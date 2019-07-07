#pragma once

#include <bnl/base/string_view.hpp>
#include <bnl/util/export.hpp>

#include <cstddef>
#include <cstdint>

namespace bnl {
namespace util {

BNL_UTIL_EXPORT bool is_lowercase(base::string_view string);

} // namespace util
} // namespace bnl
