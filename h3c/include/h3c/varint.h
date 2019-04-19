#pragma once

#include <h3c/export.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

H3C_EXPORT size_t varint_size(uint64_t varint);

H3C_EXPORT size_t varint_parse(const uint8_t *src, size_t size,
                               uint64_t *varint);

H3C_EXPORT size_t varint_serialize(uint8_t *dest, size_t size, uint64_t varint);

#ifdef __cplusplus
}
#endif
