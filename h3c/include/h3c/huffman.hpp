#pragma once

#include <h3c/export.hpp>

#include <cstddef>
#include <cstdint>
#include <system_error>

namespace h3c {

class logger;

namespace huffman {

H3C_EXPORT size_t encoded_size(const char *string, size_t size);

H3C_EXPORT std::error_code encode(uint8_t *dest,
                                  size_t size,
                                  const char *string,
                                  size_t string_size,
                                  const logger *logger);

H3C_EXPORT std::error_code decode(const uint8_t *src,
                                  size_t size,
                                  char *string,
                                  size_t *string_size,
                                  const logger *logger);

} // namespace huffman

} // namespace h3c
