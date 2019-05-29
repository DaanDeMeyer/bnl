#include <doctest/doctest.h>

#include <h3c/error.hpp>
#include <h3c/log.hpp>
#include <h3c/log/fprintf.hpp>
#include <h3c/varint.hpp>

#include <array>

static constexpr uint8_t VARINT_UINT8_HEADER = 0x00;
static constexpr uint8_t VARINT_UINT16_HEADER = 0x40;
static constexpr uint8_t VARINT_UINT32_HEADER = 0x80;
static constexpr uint8_t VARINT_UINT64_HEADER = 0xc0;

TEST_CASE("varint")
{
  h3c::logger logger;

  SUBCASE("zero")
  {
    std::array<uint8_t, sizeof(uint8_t)> dest = {};
    uint64_t n = 0;

    size_t encoded_size = h3c::varint::encoded_size(n);
    REQUIRE(encoded_size == sizeof(uint8_t));

    std::error_code error = h3c::varint::encode(dest.data(), dest.size(), n,
                                                &encoded_size, &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == sizeof(uint8_t));
    REQUIRE(dest[0] == (0x00U | VARINT_UINT8_HEADER));

    error = h3c::varint::decode(dest.data(), dest.size(), &n, &encoded_size,
                                &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == sizeof(uint8_t));
    REQUIRE(n == 0);
  }

  SUBCASE("uint8")
  {
    std::array<uint8_t, sizeof(uint8_t)> dest = {};
    uint64_t n = 62;

    size_t encoded_size = h3c::varint::encoded_size(n);
    REQUIRE(encoded_size == sizeof(uint8_t));

    std::error_code error = h3c::varint::encode(dest.data(), dest.size(), n,
                                                &encoded_size, &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == sizeof(uint8_t));
    REQUIRE(dest[0] == (0x3eU | VARINT_UINT8_HEADER));

    error = h3c::varint::decode(dest.data(), dest.size(), &n, &encoded_size,
                                &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == sizeof(uint8_t));
    REQUIRE(n == 62);
  }

  SUBCASE("uint16")
  {
    std::array<uint8_t, sizeof(uint16_t)> dest = {};
    uint64_t n = 15248;

    size_t encoded_size = h3c::varint::encoded_size(n);
    REQUIRE(encoded_size == sizeof(uint16_t));

    std::error_code error = h3c::varint::encode(dest.data(), dest.size(), n,
                                                &encoded_size, &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == sizeof(uint16_t));
    REQUIRE(dest[0] == (0x3bU | VARINT_UINT16_HEADER));
    REQUIRE(dest[1] == 0x90);

    error = h3c::varint::decode(dest.data(), dest.size(), &n, &encoded_size,
                                &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == sizeof(uint16_t));
    REQUIRE(n == 15248);
  }

  SUBCASE("uint32")
  {
    std::array<uint8_t, sizeof(uint32_t)> dest = {};
    uint64_t n = 1073721823;

    size_t encoded_size = h3c::varint::encoded_size(n);
    REQUIRE(encoded_size == sizeof(uint32_t));

    std::error_code error = h3c::varint::encode(dest.data(), dest.size(), n,
                                                &encoded_size, &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == sizeof(uint32_t));
    REQUIRE(dest[0] == (0x3fU | VARINT_UINT32_HEADER));
    REQUIRE(dest[1] == 0xff);
    REQUIRE(dest[2] == 0xb1);
    REQUIRE(dest[3] == 0xdf);

    error = h3c::varint::decode(dest.data(), dest.size(), &n, &encoded_size,
                                &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == sizeof(uint32_t));
    REQUIRE(n == 1073721823);
  }

  SUBCASE("uint64")
  {
    std::array<uint8_t, sizeof(uint64_t)> dest = {};
    uint64_t n = 4611386010427387203;

    size_t encoded_size = h3c::varint::encoded_size(n);
    REQUIRE(encoded_size == sizeof(uint64_t));

    std::error_code error = h3c::varint::encode(dest.data(), dest.size(), n,
                                                &encoded_size, &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == sizeof(uint64_t));
    REQUIRE(dest[0] == (0x3fU | VARINT_UINT64_HEADER));
    REQUIRE(dest[1] == 0xfe);
    REQUIRE(dest[2] == 0xef);
    REQUIRE(dest[3] == 0x24);
    REQUIRE(dest[4] == 0xf1);
    REQUIRE(dest[5] == 0xba);
    REQUIRE(dest[6] == 0xed);
    REQUIRE(dest[7] == 0x43);

    error = h3c::varint::decode(dest.data(), dest.size(), &n, &encoded_size,
                                &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == sizeof(uint64_t));
    REQUIRE(n == 4611386010427387203);
  }

  SUBCASE("max")
  {
    std::array<uint8_t, sizeof(uint64_t)> dest = {};
    uint64_t n = 4611686018427387903;

    size_t encoded_size = h3c::varint::encoded_size(n);
    REQUIRE(encoded_size == sizeof(uint64_t));

    std::error_code error = h3c::varint::encode(dest.data(), dest.size(), n,
                                                &encoded_size, &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == sizeof(uint64_t));
    REQUIRE(dest[0] == (0x3fU | VARINT_UINT64_HEADER));
    REQUIRE(dest[1] == 0xff);
    REQUIRE(dest[2] == 0xff);
    REQUIRE(dest[3] == 0xff);
    REQUIRE(dest[4] == 0xff);
    REQUIRE(dest[5] == 0xff);
    REQUIRE(dest[6] == 0xff);
    REQUIRE(dest[7] == 0xff);

    error = h3c::varint::decode(dest.data(), dest.size(), &n, &encoded_size,
                                &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == sizeof(uint64_t));
    REQUIRE(n == 4611686018427387903);
  }

  SUBCASE("encode: overflow")
  {
    std::array<uint8_t, sizeof(uint64_t)> dest = {};
    uint64_t n = UINT64_MAX;

    size_t encoded_size = h3c::varint::encoded_size(n);
    REQUIRE(encoded_size == 0);

    std::error_code error = h3c::varint::encode(dest.data(), dest.size(), n,
                                                &encoded_size, &logger);

    REQUIRE(error == h3c::error::varint_overflow);
    REQUIRE(encoded_size == 0);
  }

  SUBCASE("encode: buffer too small")
  {
    std::array<uint8_t, sizeof(uint8_t)> dest = {};
    uint64_t n = 169;

    size_t encoded_size = 0;
    std::error_code error = h3c::varint::encode(dest.data(), dest.size(), n,
                                                &encoded_size, &logger);

    REQUIRE(error == h3c::error::buffer_too_small);
    REQUIRE(encoded_size == 0);
  }

  SUBCASE("decode: incomplete")
  {
    std::array<uint8_t, sizeof(uint16_t)> dest = {};
    uint64_t n = 169;

    size_t encoded_size = 0;
    std::error_code error = h3c::varint::encode(dest.data(), dest.size(), n,
                                                &encoded_size, &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == sizeof(uint16_t));

    error = h3c::varint::decode(dest.data(), dest.size() - 1, &n, &encoded_size,
                                &logger);

    REQUIRE(error == h3c::error::incomplete);
    REQUIRE(encoded_size == 0);
  }
}