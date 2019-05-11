#pragma once

#include <h3c/error.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct h3c_log_t h3c_log_t;

H3C_EXPORT size_t h3c_huffman_encoded_size(const char *string, size_t size);

H3C_EXPORT H3C_ERROR h3c_huffman_encode(uint8_t *dest,
                                        size_t size,
                                        const char *string,
                                        size_t string_size,
                                        h3c_log_t *log);

H3C_EXPORT H3C_ERROR h3c_huffman_decode(const uint8_t *src,
                                        size_t size,
                                        char *string,
                                        size_t *string_size,
                                        h3c_log_t *log);

#ifdef __cplusplus
}
#endif
