/*
 * nghttp3
 *
 * Copyright (c) 2019 nghttp3 contributors
 * Copyright (c) 2013 nghttp2 contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <h3c/huffman.hpp>

#include <util/error.hpp>

#include <cassert>

namespace h3c {

#include "encode_generated.cpp"

size_t huffman::encoded_size(const char *string, size_t size)
{
  assert(string);

  size_t num_bits = 0;

  for (size_t i = 0; i < size; i++) {
    num_bits += encode_table[static_cast<unsigned char>(string[i])].num_bits;
  }

  // Pad the prefix of EOS (256).
  return (num_bits + 7) / 8;
}

static std::error_code symbol_encode(uint8_t *dest,
                                     size_t size,
                                     size_t *rem_bits,
                                     const symbol &symbol,
                                     size_t *encoded_size,
                                     const logger *logger)
{
  *encoded_size = 0;
  uint8_t *begin = dest;

  uint32_t code = symbol.code;
  size_t num_bits = symbol.num_bits;

  if (size == 0) {
    THROW(error::buffer_too_small);
  }

  if (*rem_bits == 8) {
    *dest = 0;
  }

  if (num_bits <= *rem_bits) {
    *dest |= static_cast<uint8_t>(code << (*rem_bits - num_bits));
    *rem_bits -= num_bits;

  } else /* num_bits > *rem_bits */ {

    *dest++ |= static_cast<uint8_t>(code >> (num_bits - *rem_bits));
    size--;
    num_bits -= *rem_bits;

    if ((num_bits & 0x7U) != 0U) {
      // Align code to MSB byte boundary.
      code <<= 8 - (num_bits & 0x7U);
    }

    while (num_bits > 8) {
      if (size == 0) {
        THROW(error::buffer_too_small);
      }

      *dest = static_cast<uint8_t>(code >> (num_bits - (num_bits % 8)));

      dest++;
      size--;
      num_bits -= 8;
    }

    if (size == 0) {
      THROW(error::buffer_too_small);
    }

    *dest = static_cast<uint8_t>(code);
    *rem_bits = 8 - num_bits;
  }

  if (*rem_bits == 0) {
    dest++;
    *rem_bits = 8;
  }

  *encoded_size = static_cast<size_t>(dest - begin);

  return {};
}

std::error_code huffman::encode(uint8_t *dest,
                                size_t size,
                                const char *string,
                                size_t string_size,
                                const logger *logger)
{
  assert(dest);
  assert(string);

  size_t rem_bits = 8;

  for (size_t i = 0; i < string_size; ++i) {
    const symbol &symbol = encode_table[static_cast<unsigned char>(string[i])];

    size_t encoded_size = 0;
    TRY(symbol_encode(dest, size, &rem_bits, symbol, &encoded_size, logger));

    dest += encoded_size;
    size -= encoded_size;
  }

  // 256 is special terminal symbol, pad with its prefix.
  if (rem_bits < 8) {
    // If rem_bits < 8, we should have at least 1 buffer space available.
    const symbol &symbol = encode_table[256];
    *dest |= static_cast<uint8_t>(symbol.code >> (symbol.num_bits - rem_bits));
  }

  return {};
}

} // namespace h3c
