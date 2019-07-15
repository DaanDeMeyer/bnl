#include <bnl/http3/codec/qpack/literal.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

#include <algorithm>

namespace bnl {
namespace http3 {
namespace qpack {
namespace literal {

decoder::decoder(const log::api *logger) noexcept
  : prefix_int_(logger)
  , huffman_(logger)
  , logger_(logger)
{
}

template<typename Sequence>
base::result<base::string>
decoder::decode(Sequence &encoded, uint8_t prefix) const
{
  typename Sequence::lookahead_type lookahead(encoded);

  CHECK(!lookahead.empty(), base::error::incomplete);

  bool is_huffman = static_cast<uint8_t>(*lookahead >> prefix) & 0x01; // NOLINT

  uint64_t literal_encoded_size = TRY(prefix_int_.decode(lookahead, prefix));

  if (literal_encoded_size > lookahead.size()) {
    THROW(base::error::incomplete);
  }

  size_t bounded_encoded_size = static_cast<size_t>(literal_encoded_size);

  base::string literal;

  if (is_huffman) {
    literal = TRY(huffman_.decode(lookahead, bounded_encoded_size));
  } else {
    literal.resize(bounded_encoded_size);

    for (size_t i = 0; i < bounded_encoded_size; i++) {
      literal[i] = static_cast<char>(lookahead[i]);
    }

    lookahead.consume(bounded_encoded_size);
  }

  encoded.consume(lookahead.consumed());

  return literal;
}

BNL_BASE_SEQUENCE_IMPL(BNL_HTTP3_QPACK_LITERAL_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_IMPL(BNL_HTTP3_QPACK_LITERAL_DECODE_IMPL);

} // namespace literal
} // namespace qpack
} // namespace http3
} // namespace bnl
