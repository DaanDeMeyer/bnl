#pragma once

#include <cstddef>
#include <cstdint>

namespace h3c {

struct header {
  struct {
    const char *data;
    size_t size;
  } name;

  struct {
    const char *data;
    size_t size;
  } value;
};

struct message {
  struct {
    header *data;
    size_t size;
  } headers;

  struct {
    const uint8_t *data;
    size_t size;
  } body;
};

} // namespace h3c
