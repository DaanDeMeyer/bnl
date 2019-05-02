#pragma once

#include <h3c/error.h>
#include <h3c/export.h>

#include <stddef.h>
#include <stdint.h>

// https://quicwg.org/base-drafts/draft-ietf-quic-transport.html#integer-encoding

#define H3C_VARINT_MAX ((0x40ULL << 56) - 1)

#define H3C_VARINT_UINT8_SIZE 1
#define H3C_VARINT_UINT16_SIZE 2
#define H3C_VARINT_UINT32_SIZE 4
#define H3C_VARINT_UINT64_SIZE 8

#ifdef __cplusplus
extern "C" {
#endif

H3C_EXPORT H3C_ERROR h3c_varint_serialize(uint8_t *dest,
                                          size_t size,
                                          uint64_t varint,
                                          size_t *varint_size);

H3C_EXPORT H3C_ERROR h3c_varint_parse(const uint8_t *src,
                                      size_t size,
                                      uint64_t *varint,
                                      size_t *varint_size);

#ifdef __cplusplus
}
#endif
