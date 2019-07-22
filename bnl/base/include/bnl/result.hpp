#pragma once

#include <outcome/experimental/status_result.hpp>
#include <outcome/try.hpp>

namespace bnl {

namespace outcome = OUTCOME_V2_NAMESPACE::experimental;

template<typename T>
using status_code = outcome::status_code<T>;
using errc = outcome::errc;
using system_code = outcome::system_code;
using generic_code = outcome::generic_code;
using posix_code = outcome::posix_code;

template<typename T>
using result = outcome::status_result<T>;

using OUTCOME_V2_NAMESPACE::failure;
using OUTCOME_V2_NAMESPACE::success;

#define BNL_TRY OUTCOME_TRYX

}
