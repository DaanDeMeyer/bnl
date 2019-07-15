#pragma once

#include <bnl/quic/export.hpp>

#include <cstdint>
#include <system_error>

namespace bnl {
namespace quic {

enum class error : uint32_t { handshake, crypto, path_validation };

BNL_QUIC_EXPORT const std::error_category &
error_category() noexcept;

BNL_QUIC_EXPORT std::error_code
make_error_code(error error) noexcept;

} // namespace quic
} // namespace bnl

namespace std {

template<>
struct is_error_code_enum<bnl::quic::error> : true_type {
};

} // namespace std
