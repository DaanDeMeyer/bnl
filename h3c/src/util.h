#pragma once

#include <stdbool.h>
#include <stddef.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

bool is_lowercase(const char *string, size_t length);
