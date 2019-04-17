#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t varint_t;

size_t varint_size(varint_t varint);

size_t varint_parse(const uint8_t *src, size_t size, varint_t *varint);

size_t varint_serialize(uint8_t *dest, size_t size, varint_t varint);
