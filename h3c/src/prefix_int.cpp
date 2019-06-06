#include <h3c/prefix_int.hpp>

#include <util/error.hpp>

namespace h3c {

prefix_int::encoder::encoder(logger *logger) noexcept : logger_(logger) {}

size_t
prefix_int::encoder::encoded_size(uint64_t value, uint8_t prefix) const noexcept
{
  uint8_t prefix_max = static_cast<uint8_t>((1U << prefix) - 1);

  size_t encoded_size = 0;

  if (value < prefix_max) {
    encoded_size++;
  } else {
    encoded_size++;
    value -= prefix_max;

    while (value >= 128) {
      value /= 128;
      encoded_size++;
    }

    encoded_size++;
  }

  return encoded_size;
}

size_t
prefix_int::encoder::encode(uint8_t *dest, uint64_t value, uint8_t prefix) const
    noexcept
{
  uint8_t *begin = dest;

  uint8_t prefix_max = static_cast<uint8_t>((1U << prefix) - 1);

  if (value < prefix_max) {
    *dest = static_cast<uint8_t>(*dest & static_cast<uint8_t>(~prefix_max)) |
            static_cast<uint8_t>(value);
    dest++;
  } else {
    *dest++ |= prefix_max;
    value -= prefix_max;

    while (value >= 128) {
      *dest++ = static_cast<uint8_t>((value % 128) + 128);
      value /= 128;
    }

    *dest++ = static_cast<uint8_t>(value);
  }

  return static_cast<size_t>(dest - begin);
}

buffer prefix_int::encoder::encode(uint64_t value, uint8_t prefix) const
{
  size_t encoded_size = this->encoded_size(value, prefix);
  mutable_buffer dest(encoded_size);

  ASSERT(encoded_size == encode(dest.data(), value, prefix));

  return std::move(dest);
}

prefix_int::decoder::decoder(logger *logger) : logger_(logger) {}

uint8_t prefix_int::decoder::uint8_decode(buffer &src,
                                          std::error_code &ec) const noexcept
{
  DECODE_START();

  if (src.empty()) {
    DECODE_THROW(error::incomplete);
  }

  uint8_t result = *src;

  src.advance(1);

  return result;
}

uint64_t prefix_int::decoder::decode(buffer &src,
                                     uint8_t prefix,
                                     std::error_code &ec) const noexcept
{
  DECODE_START();

  uint64_t result = DECODE_TRY(uint8_decode(src, ec));

  uint8_t prefix_max = static_cast<uint8_t>((1U << prefix) - 1);
  result &= prefix_max;

  if (result >= prefix_max) {
    uint64_t offset = 0;
    uint8_t byte = 0;
    do {
      byte = DECODE_TRY(uint8_decode(src, ec));
      result += (byte & 127U) * (1U << offset);
      offset += 7;
    } while ((byte & 128U) == 128);
  }

  return result;
}

} // namespace h3c