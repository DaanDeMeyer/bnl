#pragma once

#include <util/error.hpp>

#define DECODE_START() size_t begin = src.position()

#define DECODE_THROW(err)                                                      \
  src.undo(src.position() - begin);                                            \
  THROW(err);                                                                  \
  (void) 0

#define DECODE_TRY(statement)                                                  \
  [&]() {                                                                      \
    using type = decltype(statement);                                          \
    ec = {};                                                                   \
    auto result = statement;                                                   \
    if (ec) {                                                                  \
      src.undo(src.position() - begin);                                        \
    }                                                                          \
                                                                               \
    return ec ? type{} : std::move(result);                                    \
  }();                                                                         \
  if (ec) {                                                                    \
    return {};                                                                 \
  };                                                                           \
  (void) 0
