#pragma once

#define ENDPOINT_THROW(err)                                                    \
  ec = err;                                                                    \
                                                                               \
  if (ec != error::idle) {                                                     \
    LOG_E("{}", ec.message());                                                 \
  }                                                                            \
                                                                               \
  return {};                                                                   \
  (void) 0

#define ENDPOINT_TRY(expression)                                               \
  [&]() {                                                                      \
    ec = {};                                                                   \
    return expression;                                                         \
  }();                                                                         \
  if (ec && ec != error::idle) {                                               \
    ec = ec == error::incomplete ? std::error_code{} : ec;                     \
    return {};                                                                 \
  }                                                                            \
  (void) 0
