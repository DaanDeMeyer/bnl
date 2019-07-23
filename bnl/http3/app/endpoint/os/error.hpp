#pragma once

#include <bnl/result.hpp>
#include <bnl/util/error.hpp>

#define THROW_SYSTEM(function, errno)                                          \
  {                                                                            \
    posix_code code(errno);                                               \
    LOG_E("{}: {}", #function, code.message());                                \
                                                                               \
    return code;                                                               \
  }                                                                            \
  (void) 0
