#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  struct {
    const char *data;
    size_t length;
  } name;

  struct {
    const char *data;
    size_t length;
  } value;
} h3c_header_t;

#ifdef __cplusplus
}
#endif
