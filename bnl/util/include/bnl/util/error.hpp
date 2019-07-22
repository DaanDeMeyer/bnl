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
  LOG_E("{}", make_status_code(__VA_ARGS__).message());                        \
  return __VA_ARGS__;                                                          \
  (void) 0
