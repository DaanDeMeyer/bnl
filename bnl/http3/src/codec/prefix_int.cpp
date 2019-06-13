#include <bnl/http3/codec/prefix_int.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {

prefix_int::encoder::encoder(const log::api *logger) noexcept : logger_(logger)
{}

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
  mutable_buffer encoded(encoded_size);

  ASSERT(encoded_size == encode(encoded.data(), value, prefix));

  return std::move(encoded);
}

prefix_int::decoder::decoder(const log::api *logger) : logger_(logger) {}

uint64_t prefix_int::decoder::decode(buffer &encoded,
                                     uint8_t prefix,
                                     std::error_code &ec) const noexcept
{
  return decode<buffer>(encoded, prefix, ec);
}

uint64_t prefix_int::decoder::decode(buffers &encoded,
                                     uint8_t prefix,
                                     std::error_code &ec) const noexcept
{
  return decode<buffers>(encoded, prefix, ec);
}

template <typename Sequence>
uint64_t prefix_int::decoder::decode(Sequence &encoded,
                                     uint8_t prefix,
                                     std::error_code &ec) const noexcept
{
  typename Sequence::anchor anchor(encoded);

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

  anchor.release();

  return result;
}

template <typename Sequence>
uint8_t
prefix_int::decoder::uint8_decode(Sequence &encoded, std::error_code &ec) const
    noexcept
{
  if (encoded.empty()) {
    THROW(error::incomplete);
  }

  uint8_t result = *encoded;

  encoded += 1;

  return result;
}

} // namespace http3
} // namespace bnl
