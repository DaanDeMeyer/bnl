#include <bnl/http3/codec/qpack/literal.hpp>

#include <algorithm>

namespace bnl {
namespace http3 {
namespace qpack {
namespace literal {

size_t
encoded_size(base::string_view literal, uint8_t prefix) noexcept
{
  size_t huffman_encoded_size = huffman::encoded_size(literal);
  size_t literal_encoded_size = huffman_encoded_size < literal.size()
                                  ? huffman_encoded_size
                                  : literal.size();

  return prefix_int::encoded_size(literal_encoded_size, prefix) +
         literal_encoded_size;
}

size_t
encode(uint8_t *dest, base::string_view literal, uint8_t prefix) noexcept
{
  uint8_t *begin = dest;

  size_t huffman_encoded_size = huffman::encoded_size(literal);
  size_t literal_encoded_size = huffman_encoded_size < literal.size()
                                  ? huffman_encoded_size
                                  : literal.size();

  if (literal_encoded_size < literal.size()) {
    *dest = static_cast<uint8_t>(*dest | static_cast<uint8_t>(1U << prefix));
    dest += prefix_int::encode(dest, literal_encoded_size, prefix);
    dest += huffman::encode(dest, literal);
  } else {
    dest += prefix_int::encode(dest, literal_encoded_size, prefix);
    std::copy_n(literal.data(), literal.size(), dest);
    dest += literal.size();
  }

  return static_cast<size_t>(dest - begin);
}

base::buffer
encode(base::string_view literal, uint8_t prefix)
{
  size_t encoded_size = literal::encoded_size(literal, prefix);
  base::buffer encoded(encoded_size);

  encode(encoded.data(), literal, prefix);

  return encoded;
}

}
}
}
}
