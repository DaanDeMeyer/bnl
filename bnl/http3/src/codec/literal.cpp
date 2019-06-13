#include <bnl/http3/codec/literal.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

#include <algorithm>

namespace bnl {
namespace http3 {

literal::encoder::encoder(const log::api *logger) noexcept
    : logger_(logger), prefix_int_(logger), huffman_(logger)
{}

size_t literal::encoder::encoded_size(buffer_view literal, uint8_t prefix) const
    noexcept
{
  size_t huffman_encoded_size = huffman_.encoded_size(literal);
  size_t literal_encoded_size = huffman_encoded_size < literal.size()
                                    ? huffman_encoded_size
                                    : literal.size();

  return prefix_int_.encoded_size(literal_encoded_size, prefix) +
         literal_encoded_size;
}

size_t literal::encoder::encode(uint8_t *dest,
                                buffer_view literal,
                                uint8_t prefix) const noexcept
{
  uint8_t *begin = dest;

  size_t huffman_encoded_size = huffman_.encoded_size(literal);
  size_t literal_encoded_size = huffman_encoded_size < literal.size()
                                    ? huffman_encoded_size
                                    : literal.size();

  if (literal_encoded_size < literal.size()) {
    *dest = static_cast<uint8_t>(*dest | static_cast<uint8_t>(1U << prefix));
    dest += prefix_int_.encode(dest, literal_encoded_size, prefix);
    dest += huffman_.encode(dest, literal);
  } else {
    dest += prefix_int_.encode(dest, literal_encoded_size, prefix);
    std::copy_n(literal.data(), literal.size(), dest);
    dest += literal.size();
  }

  return static_cast<size_t>(dest - begin);
}

buffer literal::encoder::encode(buffer_view literal, uint8_t prefix) const
{
  size_t encoded_size = this->encoded_size(literal, prefix);
  mutable_buffer encoded(encoded_size);

  ASSERT(encoded_size == encode(encoded.data(), literal, prefix));

  return std::move(encoded);
}

literal::decoder::decoder(const log::api *logger) noexcept
    : logger_(logger), prefix_int_(logger), huffman_(logger)
{}

buffer literal::decoder::decode(buffer &encoded,
                                uint8_t prefix,
                                std::error_code &ec) const
{
  return decode<buffer>(encoded, prefix, ec);
}

buffer literal::decoder::decode(buffers &encoded,
                                uint8_t prefix,
                                std::error_code &ec) const
{
  return decode<buffers>(encoded, prefix, ec);
}

template <typename Sequence>
buffer literal::decoder::decode(Sequence &encoded,
                                uint8_t prefix,
                                std::error_code &ec) const
{
  typename Sequence::anchor anchor(encoded);

  if (encoded.empty()) {
    THROW(error::incomplete);
  }

  bool is_huffman_encoded = (static_cast<uint8_t>(*encoded >> prefix) // NOLINT
                             & 0x01) != 0;
  size_t literal_encoded_size = TRY(
      static_cast<size_t>(prefix_int_.decode(encoded, prefix, ec)));

  buffer literal;

  if (is_huffman_encoded) {
    literal = TRY(huffman_.decode(encoded, literal_encoded_size, ec));
  } else {
    literal = encoded.slice(literal_encoded_size);
    encoded += literal_encoded_size;
  }

  anchor.release();

  return literal;
}

} // namespace http3
} // namespace bnl
