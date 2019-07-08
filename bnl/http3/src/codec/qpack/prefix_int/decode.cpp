#include <bnl/http3/codec/qpack/prefix_int.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace qpack {
namespace prefix_int {

decoder::decoder(const log::api *logger) : logger_(logger) {}

template <typename Sequence>
uint64_t decoder::decode(Sequence &encoded,
                         uint8_t prefix,
                         std::error_code &ec) const noexcept
{
  typename Sequence::lookahead_type lookahead(encoded);

  uint64_t result = TRY(uint8_decode(encoded, ec));

  uint8_t prefix_max = static_cast<uint8_t>((1U << prefix) - 1);
  result &= prefix_max;

  if (result >= prefix_max) {
    uint64_t offset = 0;
    uint8_t byte = 0;
    do {
      byte = TRY(uint8_decode(encoded, ec));
      result += (byte & 127U) * (1U << offset);
      offset += 7;
    } while ((byte & 128U) == 128);
  }

  encoded.consume(lookahead.consumed());

  return result;
}

template <typename Lookahead>
uint8_t decoder::uint8_decode(Lookahead &encoded, std::error_code &ec) const
    noexcept
{
  CHECK(!encoded.empty(), base::error::incomplete);

  uint8_t result = *encoded;

  encoded.consume(sizeof(uint8_t));

  return result;
}

BNL_BASE_SEQUENCE_IMPL(BNL_HTTP3_QPACK_PREFIX_INT_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_IMPL(BNL_HTTP3_QPACK_PREFIX_INT_DECODE_IMPL);

} // namespace prefix_int
} // namespace qpack
} // namespace http3
} // namespace bnl
