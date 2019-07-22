#pragma once

#include <bnl/result.hpp>
#include <bnl/util/log.hpp>

#include <cstdlib>

#define NOTREACHED()                                                           \
  LOG_E("Assertion failed: NOTREACHED()");                                     \
  std::abort();                                                                \
  (void) 0

#define TRY BNL_TRY

#define THROW(...)                                                             \
  {                                                                            \
    if (logger_) {                                                             \
      logger_->operator()(__FILE__,                                            \
                          static_cast<const char *>(__func__),                 \
                          __LINE__,                                            \
                          make_status_code(__VA_ARGS__));                      \
    }                                                                          \
                                                                               \
    return __VA_ARGS__;                                                        \
  }                                                                            \
  (void) 0
