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

#include <util/enum.hpp>
#include <util/error.hpp>

namespace h3c {

#include "decode_generated.cpp"

huffman::decoder::decoder(logger *logger) noexcept : logger_(logger) {}

buffer huffman::decoder::decode(buffer &encoded,
                                size_t encoded_size,
                                std::error_code &ec) const
{
  return decode<buffer>(encoded, encoded_size, ec);
}

buffer huffman::decoder::decode(buffers &encoded,
                                size_t encoded_size,
                                std::error_code &ec) const
{
  return decode<buffers>(encoded, encoded_size, ec);
}

template <typename Sequence>
buffer huffman::decoder::decode(Sequence &encoded,
                                size_t encoded_size,
                                std::error_code &ec) const
{
  size_t decoded_size = TRY(this->decoded_size(encoded, encoded_size, ec));
  mutable_buffer decoded(decoded_size);

  uint8_t *dest = decoded.data();
  uint8_t state = 0;

  for (size_t i = 0; i < encoded_size; ++i) {
    const node &first = decode_table[state][encoded[i] >> 4U];

    if ((first.flags & util::to_underlying(decode_flag::symbol)) != 0) {
      *dest++ = first.symbol;
    }

    const node &second = decode_table[first.state][encoded[i] & 0xfU];

    if ((second.flags & util::to_underlying(decode_flag::symbol)) != 0) {
      *dest++ = second.symbol;
    }

    state = second.state;
  }

  encoded += encoded_size;

  return std::move(decoded);
}

template <typename Sequence>
size_t huffman::decoder::decoded_size(Sequence &encoded,
                                      size_t encoded_size,
                                      std::error_code &ec) const noexcept
{
  if (encoded.size() < encoded_size) {
    THROW(error::incomplete);
  }

  size_t decoded_size = 0;
  uint8_t state = 0;
  bool accept = false;

  for (size_t i = 0; i < encoded_size; i++) {
    const node &first = decode_table[state][encoded[i] >> 4U];

    if ((first.flags & util::to_underlying(decode_flag::failed)) != 0) {
      THROW(error::qpack_decompression_failed);
    }

    if ((first.flags & util::to_underlying(decode_flag::symbol)) != 0) {
      decoded_size++;
    }

    const node &second = decode_table[first.state][encoded[i] & 0xfU];

    if ((second.flags & util::to_underlying(decode_flag::failed)) != 0) {
      THROW(error::qpack_decompression_failed);
    }

    if ((second.flags & util::to_underlying(decode_flag::symbol)) != 0) {
      decoded_size++;
    }

    state = second.state;
    accept = (second.flags & util::to_underlying(decode_flag::accepted)) != 0;
  }

  if (!accept) {
    THROW(error::qpack_decompression_failed);
  }

  return decoded_size;
}

} // namespace h3c
