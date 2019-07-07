#include <bnl/http3/codec/varint.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

#include <bnl/base/error.hpp>

namespace bnl {
namespace http3 {
namespace varint {

decoder::decoder(const log::api *logger) noexcept : logger_(logger) {}

uint64_t decoder::decode(base::buffer &encoded, std::error_code &ec) const
    noexcept
{
  base::buffer::lookahead lookahead(encoded);

  uint64_t varint = TRY(decode<base::buffer::lookahead>(lookahead, ec));
  encoded.consume(lookahead.consumed());

  return varint;
}

uint64_t decoder::decode(base::buffers &encoded, std::error_code &ec) const
    noexcept
{
  base::buffers::lookahead view(encoded);

  uint64_t varint = TRY(decode<base::buffers::lookahead>(view, ec));
  encoded.consume(view.consumed());

  return varint;
}

uint64_t decoder::decode(base::buffer::lookahead &encoded,
                         std::error_code &ec) const noexcept
{
  return decode<base::buffer::lookahead>(encoded, ec);
}

uint64_t decoder::decode(base::buffers::lookahead &encoded,
                         std::error_code &ec) const noexcept
{
  return decode<base::buffers::lookahead>(encoded, ec);
}

// All decode functions convert from network to host byte order and remove the
// varint header (first two bits) before returning a value.

template <typename Lookahead>
static uint8_t uint8_decode(Lookahead &encoded)
{
  uint8_t result = encoded[0] & 0x3fU;

  encoded.consume(sizeof(uint8_t));

  return result;
}

template <typename Lookahead>
static uint16_t uint16_decode(Lookahead &encoded)
{
  uint16_t result = static_cast<uint16_t>(
      static_cast<uint16_t>(static_cast<uint16_t>(encoded[0]) << 8U) |
      static_cast<uint16_t>(static_cast<uint16_t>(encoded[1]) << 0U));

  encoded.consume(sizeof(uint16_t));

  return result & 0x3fffU;
}

template <typename Lookahead>
static uint32_t uint32_decode(Lookahead &encoded)
{

  uint32_t result = static_cast<uint32_t>(encoded[0]) << 24U |
                    static_cast<uint32_t>(encoded[1]) << 16U |
                    static_cast<uint32_t>(encoded[2]) << 8U |
                    static_cast<uint32_t>(encoded[3]) << 0U;

  encoded.consume(sizeof(uint32_t));

  return result & 0x3fffffffU;
}

template <typename Lookahead>
static uint64_t uint64_decode(Lookahead &encoded)
{
  uint64_t result = static_cast<uint64_t>(encoded[0]) << 56U |
                    static_cast<uint64_t>(encoded[1]) << 48U |
                    static_cast<uint64_t>(encoded[2]) << 40U |
                    static_cast<uint64_t>(encoded[3]) << 32U |
                    static_cast<uint64_t>(encoded[4]) << 24U |
                    static_cast<uint64_t>(encoded[5]) << 16U |
                    static_cast<uint64_t>(encoded[6]) << 8U |
                    static_cast<uint64_t>(encoded[7]) << 0U;

  encoded.consume(sizeof(uint64_t));

  return result & 0x3fffffffffffffffU;
}

template <typename Lookahead>
uint64_t decoder::decode(Lookahead &encoded, std::error_code &ec) const noexcept
{
  CHECK(!encoded.empty(), base::error::incomplete);

  size_t varint_size = 1;
  uint8_t header = static_cast<uint8_t>(*encoded >> 6U);

  // varint size = 2^header
  varint_size <<= header; // shift left => x2

  CHECK(encoded.size() >= varint_size, base::error::incomplete);

  uint64_t varint = 0;

  switch (varint_size) {
    case sizeof(uint8_t):
      varint = uint8_decode(encoded);
      break;
    case sizeof(uint16_t):
      varint = uint16_decode(encoded);
      break;
    case sizeof(uint32_t):
      varint = uint32_decode(encoded);
      break;
    case sizeof(uint64_t):
      varint = uint64_decode(encoded);
      break;
    default:
      NOTREACHED();
  }

  return varint;
}

} // namespace varint
} // namespace http3
} // namespace bnl
