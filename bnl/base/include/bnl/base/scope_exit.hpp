#pragma once

#include <type_traits>
#include <utility>

// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4189.pdf

namespace bnl {

template<typename EF>
struct scope_exit {
  explicit scope_exit(EF &&f) noexcept
    : exit_function_(std::move(f))
    , execute_on_destruction_{ true }
  {}

  scope_exit(scope_exit const &) = delete;
  void operator=(scope_exit const &) = delete;

  scope_exit(scope_exit &&rhs) noexcept
    : exit_function_(std::move(rhs.exit_function_))
    , execute_on_destruction_{ rhs.execute_on_destruction_ }
  {
    rhs.release();
  }

  scope_exit &operator=(scope_exit &&) = delete;

  ~scope_exit() noexcept(noexcept(this->exit_function_()))
  {
    if (execute_on_destruction_) {
      this->exit_function_();
    }
  }

  void release() noexcept { this->execute_on_destruction_ = false; }

private:
  EF exit_function_;
  bool execute_on_destruction_;
};

template<typename EF>
scope_exit<typename std::remove_reference<EF>::type>
make_scope_exit(EF &&exit_function) noexcept
{
  return scope_exit<EF>(std::forward<EF>(exit_function));
}

#define SCOPE_EXIT_IMPL_1(x, y) x##y
#define SCOPE_EXIT_IMPL_2(x, y) SCOPE_EXIT_IMPL_1(x, y)
#define SCOPE_EXIT_IMPL_3(x) SCOPE_EXIT_IMPL_2(x, __COUNTER__)

#define SCOPE_EXIT(code)                                                       \
  auto SCOPE_EXIT_IMPL_3(_scope_exit_) = make_scope_exit([&]() { code; })

} // namespace bnl
