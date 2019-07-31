#pragma once

#include <outcome-basic.hpp>

namespace bnl {
namespace base {

template<typename T,
         typename S,
         typename NoValuePolicy = OUTCOME_V2_NAMESPACE::policy::all_narrow>
using result = OUTCOME_V2_NAMESPACE::basic_result<T, S, NoValuePolicy>;

using OUTCOME_V2_NAMESPACE::failure;
using OUTCOME_V2_NAMESPACE::success;

#define BNL_TRY OUTCOME_TRYX

}
}
