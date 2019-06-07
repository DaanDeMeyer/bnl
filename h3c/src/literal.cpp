#include <h3c/literal.hpp>

#include <util/error.hpp>

#include <algorithm>

namespace h3c {

literal::encoder::encoder(logger *logger) noexcept
    : logger_(logger), prefix_int_(logger), huffman_(logger)
{}

size_t literal::encoder::encoded_size(const buffer &literal) const noexcept
{
  size_t huffman_encoded_size = huffman_.encoded_size(literal);
  return huffman_encoded_size < literal.size() ? huffman_encoded_size
                                               : literal.size();
}

size_t literal::encoder::encode(uint8_t *dest,
                                const buffer &literal,
                                uint8_t prefix) const noexcept
{
  uint8_t *begin = dest;

  size_t encoded_size = this->encoded_size(literal);

  if (encoded_size < literal.size()) {
    *dest |= static_cast<uint8_t>(1U << prefix);
    dest += prefix_int_.encode(dest, encoded_size, prefix);
    dest += huffman_.encode(dest, literal);
  } else {
    dest += prefix_int_.encode(dest, encoded_size, prefix);
    std::copy_n(literal.data(), literal.size(), dest);
    dest += literal.size();
  }

  return static_cast<size_t>(dest - begin);
}

buffer literal::encoder::encode(const buffer &literal, uint8_t prefix) const
{
  size_t encoded_size = this->encoded_size(literal);

  if (encoded_size > literal.size()) {
    return literal;
  }

  mutable_buffer encoded(encoded_size);

  ASSERT(encoded_size == encode(encoded.data(), literal, prefix));

  return std::move(encoded);
}

literal::decoder::decoder(logger *logger) noexcept
    : logger_(logger), prefix_int_(logger), huffman_(logger)
{}

buffer literal::decoder::decode(buffer &encoded,
                                uint8_t prefix,
                                std::error_code &ec) const
{
  DECODE_START();

  if (encoded.empty()) {
    DECODE_THROW(error::incomplete);
  }

  bool is_huffman_encoded = (static_cast<uint8_t>(*encoded >> prefix) // NOLINT
                             & 0x01) != 0;
  size_t literal_encoded_size = DECODE_TRY(
      static_cast<size_t>(prefix_int_.decode(encoded, prefix, ec)));

  if (is_huffman_encoded) {
    return DECODE_TRY(huffman_.decode(encoded, literal_encoded_size, ec));
  }

  buffer literal = encoded.slice(literal_encoded_size);
  encoded.advance(literal_encoded_size);
  return literal;
}

} // namespace h3c
