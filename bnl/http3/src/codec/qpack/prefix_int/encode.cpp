#include <bnl/http3/codec/qpack/prefix_int.hpp>

#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace qpack {
namespace prefix_int {

encoder::encoder(const log::api *logger) noexcept
  : logger_(logger)
{}

size_t
encoder::encoded_size(uint64_t value, uint8_t prefix) const noexcept
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
encoder::encode(uint8_t *dest, uint64_t value, uint8_t prefix) const noexcept
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

base::buffer
encoder::encode(uint64_t value, uint8_t prefix) const
{
  size_t encoded_size = this->encoded_size(value, prefix);
  base::buffer encoded(encoded_size);

  ASSERT(encoded_size == encode(encoded.data(), value, prefix));

  return encoded;
}

} // namespace prefix_int
} // namespace qpack
} // namespace http3
} // namespace bnl
