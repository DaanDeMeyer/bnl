#pragma once

#define H3C_MOVE_ONLY(name)                                                    \
  name(const name &) = delete;            /* NOLINT */                         \
  name &operator=(const name &) = delete; /* NOLINT */                         \
                                                                               \
  name(name &&) = default;            /* NOLINT */                             \
  name &operator=(name &&) = default; /* NOLINT */                             \
                                                                               \
  ~name() = default;
