#pragma once

#define H3C_MOVE_ONLY(name)                                                    \
  name(const name &) = delete;            /* NOLINT */                         \
  name &operator=(const name &) = delete; /* NOLINT */                         \
                                                                               \
  name(name &&) = default;           /* NOLINT */                              \
  name &operator=(name &&) = default /* NOLINT */

#define H3C_DEFAULT_COPY(name)                                                 \
  name(const name &) = default;           /* NOLINT */                         \
  name &operator=(const name &) = default /* NOLINT */

#define H3C_NO_COPY(name)                                                      \
  name(const name &) = delete;           /* NOLINT */                          \
  name &operator=(const name &) = delete /* NOLINT */

#define H3C_DEFAULT_MOVE(name)                                                 \
  name(name &&) = default;           /* NOLINT */                              \
  name &operator=(name &&) = default /* NOLINT */

#define H3C_NO_MOVE(name)                                                      \
  name(name &&) = delete;           /* NOLINT */                               \
  name &operator=(name &&) = delete /* NOLINT */
