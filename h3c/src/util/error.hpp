#pragma once

#include <h3c/error.hpp>
#include <util/log.hpp>

#include <cstdlib>

#define THROW_VOID(err)                                                        \
  {                                                                            \
    ec = err;                                                                  \
                                                                               \
    LOG_E("{}", ec.message());                                                 \
                                                                               \
    if (ec == error::internal_error) {                                         \
      ASSERT(false);                                                           \
    }                                                                          \
  }                                                                            \
  (void) 0

#define THROW(err)                                                             \
  THROW_VOID(err);                                                             \
  return {};                                                                   \
  (void) 0

#define TRY(expression)                                                        \
  [&]() {                                                                      \
    using type = decltype(expression);                                         \
    ec = {};                                                                   \
    auto result = expression;                                                  \
    return ec ? type{} : result;                                               \
  }();                                                                         \
  if (ec) {                                                                    \
    return {};                                                                 \
  };                                                                           \
  (void) 0

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
