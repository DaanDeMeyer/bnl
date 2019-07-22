#pragma once

#include <type_traits>

namespace bnl {
namespace base {

template<typename Fn>
class function_view;

template<typename Return, typename... Params>
class function_view<Return(Params...)> {
public:
  function_view() = default;

  template<typename Callable>
  function_view( // NOLINT
    Callable &&callable,
    typename std::enable_if<
      !std::is_same<typename std::remove_reference<Callable>::type,
                    function_view>::value>::type * = nullptr) // NOLINT
    : callback_(callback_fn<typename std::remove_reference<Callable>::type>)
    , callable_(reinterpret_cast<void *>(&callable))
  {}

  function_view(const function_view &) = default;
  function_view &operator=(const function_view &) = default;

  Return operator()(Params... params) const
  {
    return callback_(callable_, static_cast<Params &&>(params)...);
  }

  operator bool() const // NOLINT
  {
    return callback_;
  }

private:
  Return (*callback_)(void *callable, Params... params) = nullptr;
  void *callable_ = nullptr;

  template<typename Callable>
  static Return callback_fn(void *callable, Params... params)
  {
    return (*reinterpret_cast<Callable *>(callable))(
      static_cast<Params &&>(params)...);
  }
};

}
}
