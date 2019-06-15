#pragma once

#include <utility>

template <typename Fn>
class function_ref;

template <typename Return, typename... Params>
class function_ref<Return(Params...)> {
public:
  function_ref() = default;
  function_ref(std::nullptr_t) {}

  template <typename Callable>
  function_ref(Callable &&callable,
               typename std::enable_if<
                   !std::is_same<typename std::remove_reference<Callable>::type,
                                 function_ref>::value>::type * = nullptr)
      : callback(callback_fn<typename std::remove_reference<Callable>::type>),
        callable(reinterpret_cast<void *>(&callable))
  {}

  Return operator()(Params... params) const
  {
    return callback(callable, std::forward<Params>(params)...);
  }

  operator bool() const
  {
    return callback;
  }

private:
  Return (*callback)(void *callable, Params... params) = nullptr;
  void *callable;

  template <typename Callable>
  static Return callback_fn(void *callable, Params... params)
  {
    return (*reinterpret_cast<Callable *>(callable))(
        std::forward<Params>(params)...);
  }
};
