#pragma once

#include <bnl/result.hpp>

#define THROW_SYSTEM(function, errno)                                          \
  {                                                                            \
    posix_code code(errno);                                                    \
    return code;                                                               \
  }                                                                            \
  (void) 0
