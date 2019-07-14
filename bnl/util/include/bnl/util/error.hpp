#pragma once

#include <bnl/base/result.hpp>
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

#define TRY BNL_TRY

#define THROW(err)                                                             \
  {                                                                            \
    std::error_code err_ = err;                                                \
                                                                               \
    if (logger_) {                                                             \
      logger_->operator()(__FILE__, static_cast<const char *>(__func__),       \
                          __LINE__, err_);                                     \
    }                                                                          \
                                                                               \
    return err_;                                                               \
  }                                                                            \
  (void) 0

#define CHECK(expression, error)                                               \
  {                                                                            \
    auto result_ = expression;                                                 \
    if (!result_) {                                                            \
      THROW(error);                                                            \
    }                                                                          \
  }                                                                            \
  (void) 0
