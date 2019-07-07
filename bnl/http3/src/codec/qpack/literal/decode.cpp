#include <bnl/http3/codec/qpack/literal.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

#include <bnl/error.hpp>

#include <algorithm>

namespace bnl {
namespace http3 {
namespace qpack {
namespace literal {

decoder::decoder(const log::api *logger) noexcept
    : logger_(logger), prefix_int_(logger), huffman_(logger)
{}

string decoder::decode(buffer::lookahead &encoded,
                       uint8_t prefix,
                       std::error_code &ec) const
{
  return decode<buffer::lookahead>(encoded, prefix, ec);
}

string decoder::decode(buffers::lookahead &encoded,
                       uint8_t prefix,
                       std::error_code &ec) const
{
  return decode<buffers::lookahead>(encoded, prefix, ec);
}

template <typename Lookahead>
string decoder::decode(Lookahead &encoded,
                       uint8_t prefix,
                       std::error_code &ec) const
{
  CHECK(!encoded.empty(), base::error::incomplete);

  bool is_huffman = static_cast<uint8_t>(*encoded >> prefix) & 0x01; // NOLINT
  uint64_t literal_encoded_size = TRY(prefix_int_.decode(encoded, prefix, ec));

  if (literal_encoded_size > encoded.size()) {
    THROW(base::error::incomplete);
  }

  size_t bounded_encoded_size = static_cast<size_t>(literal_encoded_size);

  string literal;

  if (is_huffman) {
    literal = huffman_.decode(encoded, bounded_encoded_size, ec);
  } else {
    literal.resize(bounded_encoded_size);

    for (size_t i = 0; i < bounded_encoded_size; i++) {
      literal[i] = static_cast<char>(encoded[i]);
    }

    encoded.consume(bounded_encoded_size);
  }

  return literal;
}

} // namespace literal
} // namespace qpack
} // namespace http3
} // namespace bnl
