#pragma once

#include <h3c/error.hpp>
#include <h3c/log.hpp>

#define THROW(err)                                                             \
  {                                                                            \
    ec = err;                                                                  \
                                                                               \
    H3C_LOG_ERROR(&logger, "{}", ec.message());                                \
                                                                               \
    if (err == h3c::error::internal_error) {                                   \
      std::abort();                                                            \
    }                                                                          \
                                                                               \
    return {};                                                                 \
  }                                                                            \
  (void) 0

#define TRY(expression)                                                        \
  [&]() {                                                                      \
    using type = decltype(expression);                                         \
    ec = {};                                                                   \
    auto result = expression;                                                  \
    return ec ? type{} : std::move(result);                                    \
  }();                                                                         \
  if (ec) {                                                                    \
    return ec.value();                                                         \
  };                                                                           \
  (void) 0
