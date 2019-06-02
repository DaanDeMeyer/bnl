#include <h3c/varint.hpp>

#include <util/error.hpp>

#include <cassert>

namespace h3c {

varint::decoder::decoder(logger *logger) noexcept : logger_(logger)
{}

// All decode functions convert from network to host byte order and remove the
// varint header (first two bits) before returning a value.

static uint8_t uint8_decode(const uint8_t *src)
{
  return src[0] & 0x3fU;
}

static uint16_t uint16_decode(const uint8_t *src)
{
  uint16_t result = static_cast<uint16_t>(
      static_cast<uint16_t>(static_cast<uint16_t>(src[0]) << 8U) |
      static_cast<uint16_t>(static_cast<uint16_t>(src[1]) << 0U));
  return result & 0x3fffU;
}

static uint32_t uint32_decode(const uint8_t *src)
{

  uint32_t result = static_cast<uint32_t>(src[0]) << 24U |
                    static_cast<uint32_t>(src[1]) << 16U |
                    static_cast<uint32_t>(src[2]) << 8U |
                    static_cast<uint32_t>(src[3]) << 0U;

  return result & 0x3fffffffU;
}

static uint64_t uint64_decode(const uint8_t *src)
{
  uint64_t result = static_cast<uint64_t>(src[0]) << 56U |
                    static_cast<uint64_t>(src[1]) << 48U |
                    static_cast<uint64_t>(src[2]) << 40U |
                    static_cast<uint64_t>(src[3]) << 32U |
                    static_cast<uint64_t>(src[4]) << 24U |
                    static_cast<uint64_t>(src[5]) << 16U |
                    static_cast<uint64_t>(src[6]) << 8U |
                    static_cast<uint64_t>(src[7]) << 0U;

  return result & 0x3fffffffffffffffU;
}

std::error_code varint::decoder::decode(const uint8_t *src,
                                        size_t size,
                                        uint64_t *varint,
                                        size_t *encoded_size) const noexcept
{
  assert(src);
  assert(varint);
  assert(encoded_size);

  *encoded_size = 0;

  if (size == 0) {
    THROW(error::incomplete);
  }

  size_t varint_size = 1;
  uint8_t header = static_cast<uint8_t>(*src >> 6U);

  // varint size = 2^header
  varint_size <<= header; // shift left => x2

  if (varint_size > size) {
    THROW(error::incomplete);
  }

  switch (varint_size) {
    case sizeof(uint8_t):
      *varint = uint8_decode(src);
      break;
    case sizeof(uint16_t):
      *varint = uint16_decode(src);
      break;
    case sizeof(uint32_t):
      *varint = uint32_decode(src);
      break;
    case sizeof(uint64_t):
      *varint = uint64_decode(src);
      break;
    default:
      THROW(error::internal_error);
  }

  *encoded_size = varint_size;

  return {};
}

} // namespace h3c
