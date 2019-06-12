#include <h3c/varint.hpp>

#include <h3c/buffers.hpp>

#include <util/error.hpp>

namespace h3c {

varint::decoder::decoder(logger *logger) noexcept : logger_(logger) {}

uint64_t
varint::decoder::decode(buffer &encoded, std::error_code &ec) const noexcept
{
  return decode<buffer>(encoded, ec);
}

uint64_t
varint::decoder::decode(buffers &encoded, std::error_code &ec) const noexcept
{
  return decode<buffers>(encoded, ec);
}

// All decode functions convert from network to host byte order and remove the
// varint header (first two bits) before returning a value.

template <typename Sequence>
static uint8_t uint8_decode(Sequence &encoded)
{
  uint8_t result = encoded[0] & 0x3fU;

  encoded += sizeof(uint8_t);

  return result;
}

template <typename Sequence>
static uint16_t uint16_decode(Sequence &encoded)
{
  uint16_t result = static_cast<uint16_t>(
      static_cast<uint16_t>(static_cast<uint16_t>(encoded[0]) << 8U) |
      static_cast<uint16_t>(static_cast<uint16_t>(encoded[1]) << 0U));

  encoded += sizeof(uint16_t);

  return result & 0x3fffU;
}

template <typename Sequence>
static uint32_t uint32_decode(Sequence &encoded)
{

  uint32_t result = static_cast<uint32_t>(encoded[0]) << 24U |
                    static_cast<uint32_t>(encoded[1]) << 16U |
                    static_cast<uint32_t>(encoded[2]) << 8U |
                    static_cast<uint32_t>(encoded[3]) << 0U;

  encoded += sizeof(uint32_t);

  return result & 0x3fffffffU;
}

template <typename Sequence>
static uint64_t uint64_decode(Sequence &encoded)
{
  uint64_t result = static_cast<uint64_t>(encoded[0]) << 56U |
                    static_cast<uint64_t>(encoded[1]) << 48U |
                    static_cast<uint64_t>(encoded[2]) << 40U |
                    static_cast<uint64_t>(encoded[3]) << 32U |
                    static_cast<uint64_t>(encoded[4]) << 24U |
                    static_cast<uint64_t>(encoded[5]) << 16U |
                    static_cast<uint64_t>(encoded[6]) << 8U |
                    static_cast<uint64_t>(encoded[7]) << 0U;

  encoded += sizeof(uint64_t);

  return result & 0x3fffffffffffffffU;
}

template <typename Sequence>
uint64_t
varint::decoder::decode(Sequence &encoded, std::error_code &ec) const
    noexcept
{
  if (encoded.empty()) {
    THROW(error::incomplete);
  }

  size_t varint_size = 1;
  uint8_t header = static_cast<uint8_t>(*encoded >> 6U);

  // varint size = 2^header
  varint_size <<= header; // shift left => x2

  if (varint_size > encoded.size()) {
    THROW(error::incomplete);
  }

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

} // namespace h3c
