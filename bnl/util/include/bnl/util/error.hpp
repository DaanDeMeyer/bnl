#pragma once

#include <bnl/util/log.hpp>

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
    logger_->operator()(__FILE__, static_cast<const char *>(__func__),         \
                        __LINE__, ec);                                         \
  }                                                                            \
                                                                               \
  return {};                                                                   \
  (void) 0
