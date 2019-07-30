#include <bnl/http3/codec/qpack/prefix_int.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>

namespace bnl {
namespace http3 {
namespace qpack {
namespace prefix_int {

template<typename Lookahead>
result<uint8_t>
uint8_decode(Lookahead &encoded) noexcept
{
  if (encoded.empty()) {
    return base::error::incomplete;
  }

  uint8_t result = *encoded;

  encoded.consume(sizeof(uint8_t));

  return result;
}

template<typename Sequence>
result<uint64_t>
decode(Sequence &encoded, uint8_t prefix) noexcept
{
  typename Sequence::lookahead_type lookahead(encoded);

  uint64_t result = BNL_TRY(uint8_decode(encoded));

  uint8_t prefix_max = static_cast<uint8_t>((1U << prefix) - 1);
  result &= prefix_max;

  if (result >= prefix_max) {
    uint64_t offset = 0;
    uint8_t byte = 0;
    do {
      byte = BNL_TRY(uint8_decode(encoded));
      result += (byte & 127U) * (1U << offset);
      offset += 7;
    } while ((byte & 128U) == 128);
  }

  encoded.consume(lookahead.consumed());

  return result;
}

BNL_BASE_SEQUENCE_IMPL(BNL_HTTP3_QPACK_PREFIX_INT_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_IMPL(BNL_HTTP3_QPACK_PREFIX_INT_DECODE_IMPL);

}
}
}
}
