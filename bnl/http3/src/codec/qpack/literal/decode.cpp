#include <bnl/http3/codec/qpack/literal.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

#include <algorithm>

namespace bnl {
namespace http3 {
namespace qpack {

literal::decoder::decoder(const log::api *logger) noexcept
    : logger_(logger), prefix_int_(logger), huffman_(logger)
{}

buffer literal::decoder::decode(buffer_view &encoded,
                                uint8_t prefix,
                                std::error_code &ec) const
{
  return decode<buffer_view>(encoded, prefix, ec);
}

buffer literal::decoder::decode(buffers_view &encoded,
                                uint8_t prefix,
                                std::error_code &ec) const
{
  return decode<buffers_view>(encoded, prefix, ec);
}

template <typename View>
buffer literal::decoder::decode(View &encoded,
                                uint8_t prefix,
                                std::error_code &ec) const
{
  CHECK(!encoded.empty(), error::incomplete);

  bool is_huffman = static_cast<uint8_t>(*encoded >> prefix) & 0x01; // NOLINT
  uint64_t literal_encoded_size = TRY(prefix_int_.decode(encoded, prefix, ec));

  if (literal_encoded_size > encoded.size()) {
    THROW(error::incomplete);
  }

  size_t bounded_encoded_size = static_cast<size_t>(literal_encoded_size);

  buffer literal;

  if (is_huffman) {
    literal = huffman_.decode(encoded, bounded_encoded_size, ec);
  } else {
    literal = encoded.copy(bounded_encoded_size);
    encoded.consume(bounded_encoded_size);
  }

  return literal;
}

} // namespace qpack
} // namespace http3
} // namespace bnl
