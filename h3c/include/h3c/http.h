#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  struct {
    const char *data;
    size_t size;
  } name;

  struct {
    const char *data;
    size_t size;
  } value;
} h3c_header_t;

#ifdef __cplusplus
}
#endif
