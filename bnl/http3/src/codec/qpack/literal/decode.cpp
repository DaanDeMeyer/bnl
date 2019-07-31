#include <bnl/http3/codec/qpack/literal.hpp>

namespace bnl {
namespace http3 {
namespace qpack {
namespace literal {

template<typename Sequence>
result<base::string>
decode(Sequence &encoded, uint8_t prefix)
{
  typename Sequence::lookahead_type lookahead(encoded);

  if (lookahead.empty()) {
    return error::incomplete;
  }

  bool is_huffman = static_cast<uint8_t>(*lookahead >> prefix) & 0x01; // NOLINT

  uint64_t literal_encoded_size =
    BNL_TRY(prefix_int::decode(lookahead, prefix));

  if (literal_encoded_size > lookahead.size()) {
    return error::incomplete;
  }

  size_t bounded_encoded_size = static_cast<size_t>(literal_encoded_size);

  base::string literal;

  if (is_huffman) {
    literal = BNL_TRY(huffman::decode(lookahead, bounded_encoded_size));
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

}
}
}
}
