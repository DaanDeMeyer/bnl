#include <bnl/http3/codec/qpack/prefix_int.hpp>

#include <bnl/http3/error.hpp>

#include <bnl/util/error.hpp>

namespace bnl {
namespace http3 {
namespace qpack {
namespace prefix_int {

decoder::decoder(const log::api *logger) : logger_(logger) {}

uint64_t decoder::decode(buffer::lookahead &encoded,
                         uint8_t prefix,
                         std::error_code &ec) const noexcept
{
  return decode<buffer::lookahead>(encoded, prefix, ec);
}

uint64_t decoder::decode(buffers::lookahead &encoded,
                         uint8_t prefix,
                         std::error_code &ec) const noexcept
{
  return decode<buffers::lookahead>(encoded, prefix, ec);
}

template <typename Lookahead>
uint64_t decoder::decode(Lookahead &encoded,
                         uint8_t prefix,
                         std::error_code &ec) const noexcept
{
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

  return result;
}

template <typename Lookahead>
uint8_t decoder::uint8_decode(Lookahead &encoded, std::error_code &ec) const
    noexcept
{
  CHECK(!encoded.empty(), error::incomplete);

  uint8_t result = *encoded;

  encoded.consume(sizeof(uint8_t));

  return result;
}

} // namespace prefix_int
} // namespace qpack
} // namespace http3
} // namespace bnl
