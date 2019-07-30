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

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>

#include "decode_generated.cpp"

namespace bnl {
namespace http3 {
namespace qpack {
namespace huffman {

template<typename Lookahead>
result<size_t>
decoded_size(const Lookahead &encoded, size_t encoded_size) noexcept
{
  if (encoded.size() < encoded_size) {
    return base::error::incomplete;
  }

  size_t decoded_size = 0;
  uint8_t state = 0;
  bool accept = false;

  for (size_t i = 0; i < encoded_size; i++) {
    const decoding::node &first = decoding::table[state][encoded[i] >> 4U];

    bool failed = (first.flags & decoding::flag::failed) != 0;
    if (failed) {
      return connection::error::qpack_decompression_failed;
    }

    if ((first.flags & decoding::flag::symbol) != 0) {
      decoded_size++;
    }

    const decoding::node &second =
      decoding::table[first.state][encoded[i] & 0xfU];

    failed = (second.flags & decoding::flag::failed) != 0;
    if (failed) {
      return connection::error::qpack_decompression_failed;
    }

    if ((second.flags & decoding::flag::symbol) != 0) {
      decoded_size++;
    }

    state = second.state;
    accept = (second.flags & decoding::flag::accepted) != 0;
  }

  if (!accept) {
    return connection::error::qpack_decompression_failed;
  }

  return decoded_size;
}

template<typename Sequence>
result<base::string>
decode(Sequence &encoded, size_t encoded_size)
{
  size_t decoded_size = BNL_TRY(huffman::decoded_size(encoded, encoded_size));
  base::string decoded;
  decoded.resize(decoded_size);

  char *dest = &decoded[0];
  uint8_t state = 0;

  for (size_t i = 0; i < encoded_size; ++i) {
    const decoding::node &first = decoding::table[state][encoded[i] >> 4U];

    if ((first.flags & decoding::flag::symbol) != 0) {
      *dest++ = static_cast<char>(first.symbol);
    }

    const decoding::node &second =
      decoding::table[first.state][encoded[i] & 0xfU];

    if ((second.flags & decoding::flag::symbol) != 0) {
      *dest++ = static_cast<char>(second.symbol);
    }

    state = second.state;
  }

  encoded.consume(encoded_size);

  return decoded;
}

BNL_BASE_SEQUENCE_IMPL(BNL_HTTP3_QPACK_HUFFMAN_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_IMPL(BNL_HTTP3_QPACK_HUFFMAN_DECODE_IMPL);

}
}
}
}
