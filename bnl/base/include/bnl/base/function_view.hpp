#pragma once

#include <type_traits>
#include <utility>

namespace bnl {
namespace base {

template<typename Fn>
class function_view;

template<typename Return, typename... Params>
class function_view<Return(Params...)> {
public:
  function_view() = default;
  function_view(std::nullptr_t)
  {}

  template<typename Callable>
  function_view(Callable &&callable,
                typename std::enable_if<
                  !std::is_same<typename std::remove_reference<Callable>::type,
                                function_view>::value>::type * = nullptr)
    : callback_(callback_fn<typename std::remove_reference<Callable>::type>)
    , callable_(reinterpret_cast<intptr_t>(&callable))
  {}

  Return operator()(Params... params) const
  {
    return callback_(callable_, std::forward<Params>(params)...);
  }

  operator bool() const
  {
    return callback_;
  }

private:
  Return (*callback_)(intptr_t callable, Params... params) = nullptr;
  intptr_t callable_;

  template<typename Callable>
  static Return callback_fn(intptr_t callable, Params... params)
  {
    return (*reinterpret_cast<Callable *>(callable))(
      std::forward<Params>(params)...);
  }
};

}
}
