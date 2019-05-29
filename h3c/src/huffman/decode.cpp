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

namespace h3c {

#include "decode_generated.cpp"

std::error_code huffman::decode(const uint8_t *src,
                                size_t size,
                                char *string,
                                size_t *string_size,
                                const logger *logger)
{
  uint8_t state = 0;
  bool accept = false;
  size_t string_length = 0;

  for (size_t i = 0; i < size; ++i) {
    const node &first = decode_table[state][src[i] >> 4U];

    if ((first.flags & static_cast<uint8_t>(decode_flag::failed)) != 0) {
      THROW(error::qpack_decompression_failed);
    }

    if ((first.flags & static_cast<uint8_t>(decode_flag::symbol)) != 0) {
      if (*string_size == 0) {
        THROW(error::buffer_too_small);
      }

      *string++ = static_cast<char>(first.symbol);
      (*string_size)--;
      string_length++;
    }

    const node &second = decode_table[first.state][src[i] & 0xfU];

    if ((second.flags & static_cast<uint8_t>(decode_flag::failed)) != 0) {
      THROW(error::qpack_decompression_failed);
    }

    if ((second.flags & static_cast<uint8_t>(decode_flag::symbol)) != 0) {
      if (*string_size == 0) {
        THROW(error::buffer_too_small);
      }

      *string++ = static_cast<char>(second.symbol);
      (*string_size)--;
      string_length++;
    }

    state = second.state;
    accept = (second.flags & static_cast<uint8_t>(decode_flag::accepted)) != 0;
  }

  if (!accept) {
    THROW(error::qpack_decompression_failed);
  }

  *string_size = string_length;

  return {};
}

} // namespace h3c
