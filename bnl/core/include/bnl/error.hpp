#pragma once

#include <bnl/core/export.hpp>

#include <system_error>

namespace bnl {
namespace core {

enum class error {
  internal,
  not_implemented,
  invalid_argument,
  incomplete,
  idle,
  unknown
};

BNL_CORE_EXPORT const std::error_category &error_category() noexcept;

BNL_CORE_EXPORT std::error_code make_error_code(error error) noexcept;

}
} // namespace bnl

namespace std {

template <>
struct is_error_code_enum<bnl::core::error> : true_type {};

} // namespace std

namespace bnl {

template <typename State>
class state_error_handler {
public:
  state_error_handler(State &state, std::error_code &ec) noexcept
      : state_(state), ec_(ec)
  {}

  state_error_handler<State>(const state_error_handler<State> &) noexcept =
      default;
  state_error_handler<State> &operator=(
      const state_error_handler<State> &) noexcept = default;

  state_error_handler<State>(state_error_handler<State> &&) noexcept = default;
  state_error_handler<State> &operator=(
      state_error_handler<State> &&) noexcept = default;

  ~state_error_handler()
  {
    if (ec_ && ec_ != core::error::idle && ec_ != core::error::incomplete &&
        ec_ != core::error::unknown) {
      state_ = State::error;
    }
  }

private:
  State &state_;
  std::error_code &ec_;
};

} // namespace bnl
