#pragma once

#include <h3c/log.h>

#define THROW(error)                                                           \
  switch ((error)) {                                                           \
    case H3C_SUCCESS:                                                          \
      break;                                                                   \
    case H3C_ERROR_INTERNAL_ERROR:                                             \
      assert(0);                                                               \
    default:                                                                   \
      H3C_LOG_ERROR(log, "%s", h3c_error_string((error)));                     \
      break;                                                                   \
  }                                                                            \
                                                                               \
  return (error);                                                              \
  (void) 0
