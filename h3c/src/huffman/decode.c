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

#include <h3c/huffman.h>

#include <util/error.h>

#include <stdbool.h>

#include "decode_generated.c"

H3C_ERROR
h3c_huffman_decode(const uint8_t *src,
                   size_t size,
                   char *string,
                   size_t *string_size,
                   h3c_log_t *log)
{
  uint8_t state = 0;
  bool accept = 0;
  size_t string_length = 0;

  for (size_t i = 0; i < size; ++i) {
    const huffman_node_t *node = &decode_table[state][src[i] >> 4];

    if (node->flags & HUFFMAN_DECODE_FAILED) {
      THROW(H3C_ERROR_QPACK_DECOMPRESSION_FAILED);
    }

    if (node->flags & HUFFMAN_DECODE_SYMBOL) {
      if (*string_size == 0) {
        THROW(H3C_ERROR_BUFFER_TOO_SMALL);
      }

      *string++ = (char) node->symbol;
      (*string_size)--;
      string_length++;
    }

    node = &decode_table[node->state][src[i] & 0xf];

    if (node->flags & HUFFMAN_DECODE_FAILED) {
      THROW(H3C_ERROR_QPACK_DECOMPRESSION_FAILED);
    }

    if (node->flags & HUFFMAN_DECODE_SYMBOL) {
      if (*string_size == 0) {
        THROW(H3C_ERROR_BUFFER_TOO_SMALL);
      }

      *string++ = (char) node->symbol;
      (*string_size)--;
      string_length++;
    }

    state = node->state;
    accept = (node->flags & HUFFMAN_DECODE_ACCEPTED) != 0;
  }

  if (!accept) {
    THROW(H3C_ERROR_QPACK_DECOMPRESSION_FAILED);
  }

  *string_size = string_length;

  return H3C_SUCCESS;
}
