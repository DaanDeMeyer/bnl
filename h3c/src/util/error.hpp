#pragma once

#include <h3c/error.hpp>
#include <h3c/log.hpp>

#include <cassert>

#define THROW(err)                                                             \
  {                                                                            \
    std::error_code code(err);                                                 \
                                                                               \
    switch (err) {                                                             \
      case error::internal_error:                                              \
        assert(0);                                                             \
      default:                                                                 \
        H3C_LOG_ERROR(logger_, "{}", code.message());                          \
        break;                                                                 \
    }                                                                          \
                                                                               \
    return code;                                                               \
  }                                                                            \
  (void) 0

#define TRY(statement)                                                         \
  {                                                                            \
    std::error_code error = statement;                                         \
    if (error) {                                                               \
      return error;                                                            \
    }                                                                          \
  }                                                                            \
  (void) 0
