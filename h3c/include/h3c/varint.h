#pragma once

#include <h3c/export.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t varint_t;

H3C_EXPORT size_t varint_size(varint_t varint);

H3C_EXPORT size_t varint_parse(const uint8_t *src, size_t size,
                               varint_t *varint);

H3C_EXPORT size_t varint_serialize(uint8_t *dest, size_t size, varint_t varint);

#ifdef __cplusplus
}
#endif
