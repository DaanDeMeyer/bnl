#pragma once

#define BNL_BASE_NO_COPY(name)                                                 \
  name(const name&) = delete;           /* NOLINT */                           \
  name& operator=(const name&) = delete /* NOLINT */

#define BNL_BASE_DEFAULT_COPY(name)                                            \
  name(const name&) = default;           /* NOLINT */                          \
  name& operator=(const name&) = default /* NOLINT */

#define BNL_BASE_CUSTOM_COPY(name)                                             \
  name(const name&);           /* NOLINT */                                    \
  name& operator=(const name&) /* NOLINT */

#define BNL_BASE_NO_MOVE(name)                                                 \
  name(name&&) = delete;           /* NOLINT */                                \
  name& operator=(name&&) = delete /* NOLINT */

#define BNL_BASE_DEFAULT_MOVE(name)                                            \
  name(name&&) = default;           /* NOLINT */                               \
  name& operator=(name&&) = default /* NOLINT */

#define BNL_BASE_CUSTOM_MOVE(name)                                             \
  name(name&&) noexcept;           /* NOLINT */                                \
  name& operator=(name&&) noexcept /* NOLINT */

#define BNL_BASE_MOVE_ONLY(name)                                               \
  BNL_BASE_NO_COPY(name);      /* NOLINT */                                    \
  BNL_BASE_DEFAULT_MOVE(name); /* NOLINT */                                    \
  ~name() noexcept = default   /* NOLINT */

#define BNL_BASE_SEQUENCE_DECL(IMPL)                                           \
  extern IMPL(base::buffer);                                                   \
  extern IMPL(base::buffers)

#define BNL_BASE_SEQUENCE_IMPL(IMPL)                                           \
  IMPL(base::buffer);                                                          \
  IMPL(base::buffers)

#define BNL_BASE_LOOKAHEAD_DECL(IMPL)                                          \
  extern IMPL(base::buffer::lookahead);                                        \
  extern IMPL(base::buffers::lookahead)

#define BNL_BASE_LOOKAHEAD_IMPL(IMPL)                                          \
  IMPL(base::buffer::lookahead);                                               \
  IMPL(base::buffers::lookahead)
