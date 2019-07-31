#pragma once

#include <bnl/http3/result.hpp>
#include <bnl/quic/result.hpp>

#include <system_error>

namespace bnl {

namespace http3 {

BNL_BASE_EXPORT const std::error_category &error_category() noexcept;

BNL_BASE_EXPORT std::error_code make_error_code(error error) noexcept;

}

namespace quic {

BNL_BASE_EXPORT const std::error_category &error_category() noexcept;

BNL_BASE_EXPORT std::error_code make_error_code(error error) noexcept;

}

}

namespace std {

template <> struct is_error_code_enum<bnl::http3::error> : true_type {};
template <> struct is_error_code_enum<bnl::quic::error> : true_type {};

}


