#pragma once

#include <h3c/error.hpp>

#include <util/log.hpp>

#include <cstdlib>

#define ASSERT(expression)                                                     \
  {                                                                            \
    auto result = expression;                                                  \
    if (!result) {                                                             \
      LOG_E("Assertion failed: {}", #expression);                              \
      std::abort();                                                            \
    }                                                                          \
  }                                                                            \
  (void) 0

#define NOTREACHED()                                                           \
  LOG_E("Assertion failed: NOTREACHED()");                                     \
  std::abort();                                                                \
  (void) 0

#define TRY_VOID(expression)                                                   \
  [&]() {                                                                      \
    ec = {};                                                                   \
    return expression;                                                         \
  }();                                                                         \
  if (ec) {                                                                    \
    return;                                                                    \
  }                                                                            \
  (void) 0

#define TRY(expression)                                                        \
  [&]() {                                                                      \
    ec = {};                                                                   \
    return expression;                                                         \
  }();                                                                         \
  if (ec) {                                                                    \
    return {};                                                                 \
  };                                                                           \
  (void) 0

#define THROW_VOID(err)                                                        \
  ec = err;                                                                    \
                                                                               \
  LOG_E("{}", ec.message());                                                   \
                                                                               \
  return;                                                                      \
  (void) 0

#define THROW(err)                                                             \
  ec = err;                                                                    \
                                                                               \
  if (logger_) {                                                               \
    logger_->log(__FILE__, static_cast<const char *>(__func__), __LINE__, ec); \
  }                                                                            \
                                                                               \
  return {};                                                                   \
  (void) 0

namespace h3c {

template <typename State>
class state_error_handler {
public:
  state_error_handler(State &state, std::error_code &ec) noexcept
      : state_(state), ec_(ec)
  {}

  ~state_error_handler()
  {
    if (ec_ && ec_ != error::idle && ec_ != error::incomplete &&
        ec_ != error::unknown) {
      state_ = State::error;
    }
  }

private:
  State &state_;
  std::error_code &ec_;
};

} // namespace h3c
