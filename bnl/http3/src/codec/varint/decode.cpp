#include <bnl/http3/codec/varint.hpp>

#include <bnl/base/error.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace varint {

decoder::decoder(const log::api *logger) noexcept
  : logger_(logger)
{}

// All decode functions convert from network to host byte order and remove the
// varint header (first two bits) before returning a value.

template<typename Lookahead>
static uint8_t
uint8_decode(Lookahead &lookahead)
{
  uint8_t result = lookahead[0] & 0x3fU;

  lookahead.consume(sizeof(uint8_t));

  return result;
}

template<typename Lookahead>
static uint16_t
uint16_decode(Lookahead &lookahead)
{
  uint16_t result = static_cast<uint16_t>(
    static_cast<uint16_t>(static_cast<uint16_t>(lookahead[0]) << 8U) |
    static_cast<uint16_t>(static_cast<uint16_t>(lookahead[1]) << 0U));

  lookahead.consume(sizeof(uint16_t));

  return result & 0x3fffU;
}

template<typename Lookahead>
static uint32_t
uint32_decode(Lookahead &lookahead)
{

  uint32_t result = static_cast<uint32_t>(lookahead[0]) << 24U |
                    static_cast<uint32_t>(lookahead[1]) << 16U |
                    static_cast<uint32_t>(lookahead[2]) << 8U |
                    static_cast<uint32_t>(lookahead[3]) << 0U;

  lookahead.consume(sizeof(uint32_t));

  return result & 0x3fffffffU;
}

template<typename Lookahead>
static uint64_t
uint64_decode(Lookahead &lookahead)
{
  uint64_t result = static_cast<uint64_t>(lookahead[0]) << 56U |
                    static_cast<uint64_t>(lookahead[1]) << 48U |
                    static_cast<uint64_t>(lookahead[2]) << 40U |
                    static_cast<uint64_t>(lookahead[3]) << 32U |
                    static_cast<uint64_t>(lookahead[4]) << 24U |
                    static_cast<uint64_t>(lookahead[5]) << 16U |
                    static_cast<uint64_t>(lookahead[6]) << 8U |
                    static_cast<uint64_t>(lookahead[7]) << 0U;

  lookahead.consume(sizeof(uint64_t));

  return result & 0x3fffffffffffffffU;
}

template<typename Sequence>
result<uint64_t>
decoder::decode(Sequence &encoded) const noexcept
{
  typename Sequence::lookahead_type lookahead(encoded);

  if (lookahead.empty()) {
    return base::error::incomplete;
  }

  size_t varint_size = 1;
  uint8_t header = static_cast<uint8_t>(*lookahead >> 6U);

  // varint size = 2^header
  varint_size <<= header; // shift left => x2

  if (lookahead.size() < varint_size) {
    return base::error::incomplete;
  }

  uint64_t varint = 0;

  switch (varint_size) {
    case sizeof(uint8_t):
      varint = uint8_decode(lookahead);
      break;
    case sizeof(uint16_t):
      varint = uint16_decode(lookahead);
      break;
    case sizeof(uint32_t):
      varint = uint32_decode(lookahead);
      break;
    case sizeof(uint64_t):
      varint = uint64_decode(lookahead);
      break;
    default:
      NOTREACHED();
  }

  encoded.consume(lookahead.consumed());

  return varint;
}

BNL_BASE_SEQUENCE_IMPL(BNL_HTTP3_VARINT_DECODE_IMPL);
BNL_BASE_LOOKAHEAD_IMPL(BNL_HTTP3_VARINT_DECODE_IMPL);

}
}
}
