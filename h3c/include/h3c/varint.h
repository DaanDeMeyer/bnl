#pragma once

#include <h3c/export.h>

#include <stddef.h>
#include <stdint.h>

// https://quicwg.org/base-drafts/draft-ietf-quic-transport.html#integer-encoding

#define H3C_VARINT_MAX ((0x40ULL << 56) - 1)

#ifdef __cplusplus
extern "C" {
#endif

H3C_EXPORT size_t h3c_varint_size(uint64_t varint);

H3C_EXPORT size_t h3c_varint_serialize(uint8_t *dest,
                                       size_t size,
                                       uint64_t varint);

H3C_EXPORT size_t h3c_varint_parse(const uint8_t *src,
                                   size_t size,
                                   uint64_t *varint);

#ifdef __cplusplus
}
#endif
