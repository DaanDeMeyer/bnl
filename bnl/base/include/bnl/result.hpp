#pragma once

#include <outcome/experimental/status_result.hpp>
#include <outcome/try.hpp>

namespace bnl {

template<typename T>
using status_code = OUTCOME_V2_NAMESPACE::experimental::status_code<T>;
template<typename T>
using status_error = OUTCOME_V2_NAMESPACE::experimental::status_error<T>;
template<typename T>
using result = OUTCOME_V2_NAMESPACE::experimental::status_result<T>;

using status_code_domain =
  OUTCOME_V2_NAMESPACE::experimental::status_code_domain;
using errc = OUTCOME_V2_NAMESPACE::experimental::errc;
using system_code = OUTCOME_V2_NAMESPACE::experimental::system_code;
using generic_code = OUTCOME_V2_NAMESPACE::experimental::generic_code;
using posix_code = OUTCOME_V2_NAMESPACE::experimental::posix_code;

using OUTCOME_V2_NAMESPACE::failure;
using OUTCOME_V2_NAMESPACE::success;

using in_place_t = OUTCOME_V2_NAMESPACE::experimental::in_place_t;
constexpr in_place_t in_place{};

#define BNL_TRY OUTCOME_TRYX

}
