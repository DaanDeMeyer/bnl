#pragma once

// Macros for explicit template instantiation. See the http3 codec folder for
// example usage.

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
