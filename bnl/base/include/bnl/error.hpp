#pragma once

#include <bnl/base/export.hpp>

#include <system_error>

namespace bnl {
namespace base {

enum class error {
  internal,
  not_implemented,
  invalid_argument,
  incomplete,
  idle,
  unknown
};

BNL_BASE_EXPORT const std::error_category &error_category() noexcept;

BNL_BASE_EXPORT std::error_code make_error_code(error error) noexcept;

}
} // namespace bnl

namespace std {

template <>
struct is_error_code_enum<bnl::base::error> : true_type {};

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
    if (ec_ && ec_ != base::error::idle && ec_ != base::error::incomplete &&
        ec_ != base::error::unknown) {
      state_ = State::error;
    }
  }

private:
  State &state_;
  std::error_code &ec_;
};

} // namespace bnl
