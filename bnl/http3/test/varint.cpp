#include <doctest/doctest.h>

#include <bnl/http3/error.hpp>

#include <bnl/http3/codec/varint.hpp>

#include <bnl/log.hpp>
#include <bnl/error.hpp>

static constexpr uint8_t VARINT_UINT8_HEADER = 0x00;
static constexpr uint8_t VARINT_UINT16_HEADER = 0x40;
static constexpr uint8_t VARINT_UINT32_HEADER = 0x80;
static constexpr uint8_t VARINT_UINT64_HEADER = 0xc0;

using namespace bnl;

TEST_CASE("varint")
{
  log::api logger;

  http3::varint::encoder encoder(&logger);
  http3::varint::decoder decoder(&logger);

  std::error_code ec;

  SUBCASE("zero")
  {
    uint64_t varint = 0;

    size_t encoded_size = encoder.encoded_size(varint, ec);

    REQUIRE(!ec);
    REQUIRE(encoded_size == sizeof(uint8_t));

    buffer encoded = encoder.encode(varint, ec);

    REQUIRE(!ec);
    REQUIRE(encoded_size == encoded.size());
    REQUIRE(encoded[0] == (0x00U | VARINT_UINT8_HEADER));

    uint64_t decoded = decoder.decode(encoded, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.empty());
    REQUIRE(decoded == varint);
  }

  SUBCASE("uint8")
  {
    uint64_t varint = 62;

    size_t encoded_size = encoder.encoded_size(varint, ec);

    REQUIRE(!ec);
    REQUIRE(encoded_size == sizeof(uint8_t));

    buffer encoded = encoder.encode(varint, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.size() == encoded_size);
    REQUIRE(encoded[0] == (0x3eU | VARINT_UINT8_HEADER));

    uint64_t decoded = decoder.decode(encoded, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.empty());
    REQUIRE(decoded == varint);
  }

  SUBCASE("uint16")
  {
    uint64_t varint = 15248;

    size_t encoded_size = encoder.encoded_size(varint, ec);

    REQUIRE(!ec);
    REQUIRE(encoded_size == sizeof(uint16_t));

    buffer encoded = encoder.encode(varint, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.size() == encoded_size);
    REQUIRE(encoded[0] == (0x3bU | VARINT_UINT16_HEADER));
    REQUIRE(encoded[1] == 0x90);

    uint64_t decoded = decoder.decode(encoded, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.empty());
    REQUIRE(decoded == varint);
  }

  SUBCASE("uint32")
  {
    uint64_t varint = 1073721823;

    size_t encoded_size = encoder.encoded_size(varint, ec);

    REQUIRE(!ec);
    REQUIRE(encoded_size == sizeof(uint32_t));

    buffer encoded = encoder.encode(varint, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.size() == encoded_size);
    REQUIRE(encoded[0] == (0x3fU | VARINT_UINT32_HEADER));
    REQUIRE(encoded[1] == 0xff);
    REQUIRE(encoded[2] == 0xb1);
    REQUIRE(encoded[3] == 0xdf);

    uint64_t decoded = decoder.decode(encoded, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.empty());
    REQUIRE(decoded == varint);
  }

  SUBCASE("uint64")
  {
    uint64_t varint = 4611386010427387203;

    size_t encoded_size = encoder.encoded_size(varint, ec);

    REQUIRE(!ec);
    REQUIRE(encoded_size == sizeof(uint64_t));

    buffer encoded = encoder.encode(varint, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.size() == encoded_size);
    REQUIRE(encoded[0] == (0x3fU | VARINT_UINT64_HEADER));
    REQUIRE(encoded[1] == 0xfe);
    REQUIRE(encoded[2] == 0xef);
    REQUIRE(encoded[3] == 0x24);
    REQUIRE(encoded[4] == 0xf1);
    REQUIRE(encoded[5] == 0xba);
    REQUIRE(encoded[6] == 0xed);
    REQUIRE(encoded[7] == 0x43);

    uint64_t decoded = decoder.decode(encoded, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.empty());
    REQUIRE(decoded == varint);
  }

  SUBCASE("max")
  {
    uint64_t varint = http3::varint::max;

    size_t encoded_size = encoder.encoded_size(varint, ec);

    REQUIRE(!ec);
    REQUIRE(encoded_size == sizeof(uint64_t));

    buffer encoded = encoder.encode(varint, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.size() == encoded_size);
    REQUIRE(encoded[0] == (0x3fU | VARINT_UINT64_HEADER));
    REQUIRE(encoded[1] == 0xff);
    REQUIRE(encoded[2] == 0xff);
    REQUIRE(encoded[3] == 0xff);
    REQUIRE(encoded[4] == 0xff);
    REQUIRE(encoded[5] == 0xff);
    REQUIRE(encoded[6] == 0xff);
    REQUIRE(encoded[7] == 0xff);

    uint64_t decoded = decoder.decode(encoded, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.empty());
    REQUIRE(decoded == varint);
  }

  SUBCASE("encode: overflow")
  {
    uint64_t varint = http3::varint::max + 1;

    size_t encoded_size = encoder.encoded_size(varint, ec);

    REQUIRE(ec == http3::error::varint_overflow);
    REQUIRE(encoded_size == 0);

    buffer encoded = encoder.encode(varint, ec);

    REQUIRE(ec == http3::error::varint_overflow);
    REQUIRE(encoded.empty());
  }

  SUBCASE("decode: incomplete")
  {
    uint64_t varint = 169;

    buffer encoded = encoder.encode(varint, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.size() == sizeof(uint16_t));

    buffer incomplete = encoded.copy(encoded.size() - 1);

    decoder.decode(incomplete, ec);

    REQUIRE(ec == base::error::incomplete);
    REQUIRE(incomplete.size() == encoded.size() - 1);

    decoder.decode(encoded, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.empty());
  }
}
