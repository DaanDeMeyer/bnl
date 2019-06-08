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
  LOG_E("{}", ec.message());                                                   \
                                                                               \
  return {};                                                                   \
  (void) 0

#define DECODE_START() const uint8_t *begin = encoded.data()

#define DECODE_THROW(err)                                                      \
  DECODE_RESET();                                                              \
  THROW(err);                                                                  \
  (void) 0

#define DECODE_TRY(expression)                                                 \
  [&]() {                                                                      \
    ec = {};                                                                   \
                                                                               \
    auto result = expression;                                                  \
    if (ec) {                                                                  \
      DECODE_RESET();                                                          \
    }                                                                          \
                                                                               \
    return result;                                                             \
  }();                                                                         \
  if (ec) {                                                                    \
    return {};                                                                 \
  };                                                                           \
  (void) 0

#define DECODE_SIZE() static_cast<size_t>(encoded.data() - begin)

#define DECODE_COMMIT() begin = encoded.data()

#define DECODE_RESET() encoded.reset(begin)
