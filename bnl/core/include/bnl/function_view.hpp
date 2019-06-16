#pragma once

#include <utility>

template <typename Fn>
class function_view;

template <typename Return, typename... Params>
class function_view<Return(Params...)> {
public:
  function_view() = default;

  template <typename Callable>
  function_view( // NOLINT
      Callable &&callable,
      typename std::enable_if<
          !std::is_same<typename std::remove_reference<Callable>::type,
                        function_view>::value>::type * = nullptr) // NOLINT
      : callback(callback_fn<typename std::remove_reference<Callable>::type>),
        callable(reinterpret_cast<void *>(&callable))
  {}

  Return operator()(Params... params) const
  {
    return callback(callable, std::forward<Params>(params)...);
  }

  operator bool() const // NOLINT
  {
    return callback;
  }

private:
  Return (*callback)(void *callable, Params... params) = nullptr;
  void *callable = nullptr;

  template <typename Callable>
  static Return callback_fn(void *callable, Params... params)
  {
    return (*reinterpret_cast<Callable *>(callable))(
        std::forward<Params>(params)...);
  }
};
