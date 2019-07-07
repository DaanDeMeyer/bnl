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

#include <bnl/util/enum.hpp>
#include <bnl/util/error.hpp>

#include <bnl/error.hpp>

#include "decode_generated.cpp"

namespace bnl {
namespace http3 {
namespace qpack {
namespace huffman {

decoder::decoder(const log::api *logger) noexcept : logger_(logger) {}

string decoder::decode(buffer &encoded,
                            size_t encoded_size,
                            std::error_code &ec) const
{
  buffer::lookahead lookahead(encoded);

  string decoded = TRY(
      decode<buffer::lookahead>(lookahead, encoded_size, ec));
  encoded.consume(lookahead.consumed());

  return decoded;
}

string decoder::decode(buffers &encoded,
                            size_t encoded_size,
                            std::error_code &ec) const
{
  buffers::lookahead view(encoded);

  string decoded = TRY(decode<buffers::lookahead>(view, encoded_size, ec));
  encoded.consume(view.consumed());

  return decoded;
}

string decoder::decode(buffer::lookahead &encoded,
                       size_t encoded_size,
                       std::error_code &ec) const
{
  return decode<buffer::lookahead>(encoded, encoded_size, ec);
}

string decoder::decode(buffers::lookahead &encoded,
                            size_t encoded_size,
                            std::error_code &ec) const
{
  return decode<buffers::lookahead>(encoded, encoded_size, ec);
}

template <typename Lookahead>
string decoder::decode(Lookahead &encoded,
                            size_t encoded_size,
                            std::error_code &ec) const
{
  size_t decoded_size = TRY(this->decoded_size(encoded, encoded_size, ec));
  string decoded;
  decoded.resize(decoded_size);

  char *dest = &decoded[0];
  uint8_t state = 0;

  for (size_t i = 0; i < encoded_size; ++i) {
    const decode::node &first = decode::table[state][encoded[i] >> 4U];

    if ((first.flags & util::to_underlying(decode::flag::symbol)) != 0) {
      *dest++ = static_cast<char>(first.symbol);
    }

    const decode::node &second = decode::table[first.state][encoded[i] & 0xfU];

    if ((second.flags & util::to_underlying(decode::flag::symbol)) != 0) {
      *dest++ = static_cast<char>(second.symbol);
    }

    state = second.state;
  }

  encoded.consume(encoded_size);

  return decoded;
}

template <typename Lookahead>
size_t decoder::decoded_size(const Lookahead &encoded,
                             size_t encoded_size,
                             std::error_code &ec) const noexcept
{
  CHECK(encoded.size() >= encoded_size, core::error::incomplete);

  size_t decoded_size = 0;
  uint8_t state = 0;
  bool accept = false;

  for (size_t i = 0; i < encoded_size; i++) {
    const decode::node &first = decode::table[state][encoded[i] >> 4U];

    bool failed = (first.flags & decode::flag::failed) != 0;
    CHECK(!failed, error::qpack_decompression_failed);

    if ((first.flags & decode::flag::symbol) != 0) {
      decoded_size++;
    }

    const decode::node &second = decode::table[first.state][encoded[i] & 0xfU];

    failed = (second.flags & decode::flag::failed) != 0;
    CHECK(!failed, error::qpack_decompression_failed);

    if ((second.flags & decode::flag::symbol) != 0) {
      decoded_size++;
    }

    state = second.state;
    accept = (second.flags & decode::flag::accepted) != 0;
  }

  CHECK(accept, error::qpack_decompression_failed);

  return decoded_size;
}

} // namespace huffman
} // namespace qpack
} // namespace http3
} // namespace bnl
