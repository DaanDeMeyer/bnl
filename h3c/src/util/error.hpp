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

#define DECODE_START() size_t begin = encoded.position()

#define DECODE_THROW(err)                                                      \
  encoded.undo(encoded.position() - begin);                                    \
  THROW(err);                                                                  \
  (void) 0

#define DECODE_TRY(statement)                                                  \
  [&]() {                                                                      \
    using type = decltype(statement);                                          \
    ec = {};                                                                   \
                                                                               \
    auto result = statement;                                                   \
    if (ec) {                                                                  \
      encoded.undo(encoded.position() - begin);                                \
    }                                                                          \
                                                                               \
    return ec ? type{} : std::move(result);                                    \
  }();                                                                         \
  if (ec) {                                                                    \
    return {};                                                                 \
  };                                                                           \
  (void) 0

