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

#include <bnl/http3/codec/qpack/huffman.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace qpack {

#include "encode_generated.cpp"

huffman::encoder::encoder(const log::api *logger) noexcept : logger_(logger) {}

size_t huffman::encoder::encoded_size(buffer_view string) const noexcept
{
  size_t num_bits = 0;

  for (uint8_t character : string) {
    num_bits += encode_table[character].num_bits;
  }

  // Pad the prefix of EOS (256).
  return (num_bits + 7) / 8;
}

static size_t symbol_encode(uint8_t *dest,
                            size_t *rem_bits,
                            const symbol &symbol)
{
  uint8_t *begin = dest;

  uint32_t code = symbol.code;
  size_t num_bits = symbol.num_bits;

  if (*rem_bits == 8) {
    *dest = 0;
  }

  if (num_bits <= *rem_bits) {
    *dest = static_cast<uint8_t>(
        *dest | static_cast<uint8_t>(code << (*rem_bits - num_bits)));
    *rem_bits -= num_bits;

  } else /* num_bits > *rem_bits */ {
    *dest = static_cast<uint8_t>(
        *dest | static_cast<uint8_t>(code >> (num_bits - *rem_bits)));
    dest++;
    num_bits -= *rem_bits;

    if ((num_bits & 0x7U) != 0U) {
      // Align code to MSB byte boundary.
      code <<= 8 - (num_bits & 0x7U);
    }

    while (num_bits > 8) {
      *dest = static_cast<uint8_t>(code >> (num_bits - (num_bits % 8)));

      dest++;
      num_bits -= 8;
    }

    *dest = static_cast<uint8_t>(code);
    *rem_bits = 8 - num_bits;
  }

  if (*rem_bits == 0) {
    dest++;
    *rem_bits = 8;
  }

  return static_cast<size_t>(dest - begin);
}

size_t huffman::encoder::encode(uint8_t *dest, buffer_view string) const
    noexcept
{
  uint8_t *begin = dest;
  size_t rem_bits = 8;

  for (uint8_t character : string) {
    const symbol &symbol = encode_table[character];
    dest += symbol_encode(dest, &rem_bits, symbol);
  }

  // 256 is special terminal symbol, pad with its prefix.
  if (rem_bits < 8) {
    // If rem_bits < 8, we should have at least 1 buffer space available.
    const symbol &symbol = encode_table[256];
    *dest = static_cast<uint8_t>(
        *dest |
        static_cast<uint8_t>(symbol.code >> (symbol.num_bits - rem_bits)));
    dest++;
  }

  return static_cast<size_t>(dest - begin);
}

buffer huffman::encoder::encode(buffer_view string) const
{
  size_t encoded_size = this->encoded_size(string);
  buffer encoded(encoded_size);

  ASSERT(encoded_size == encode(encoded.data(), string));

  return encoded;
}

} // namespace qpack
} // namespace http3
} // namespace bnl
