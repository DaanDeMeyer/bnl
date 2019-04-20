#include <doctest.h>

#include <h3c/varint.h>

#include <array>

static constexpr uint8_t VARINT_UINT8_HEADER = 0x00;
static constexpr uint8_t VARINT_UINT16_HEADER = 0x40;
static constexpr uint8_t VARINT_UINT32_HEADER = 0x80;
static constexpr uint8_t VARINT_UINT64_HEADER = 0xc0;

TEST_CASE("varint")
{
  SUBCASE("zero")
  {
    std::array<uint8_t, 1> dest = { {} };
    uint64_t n = 0;

    size_t rv = h3c_varint_serialize(dest.data(), dest.size(), n);

    REQUIRE(rv == 1);
    REQUIRE(dest[0] == (0x00 | VARINT_UINT8_HEADER));

    rv = h3c_varint_parse(dest.data(), dest.size(), &n);

    REQUIRE(rv == 1);
    REQUIRE(n == 0);
  }

  SUBCASE("uint8")
  {
    std::array<uint8_t, 1> dest = { {} };
    uint64_t n = 62;

    size_t rv = h3c_varint_serialize(dest.data(), dest.size(), n);

    REQUIRE(rv == 1);
    REQUIRE(dest[0] == (0x3E | VARINT_UINT8_HEADER));

    rv = h3c_varint_parse(dest.data(), dest.size(), &n);

    REQUIRE(rv == 1);
    REQUIRE(n == 62);
  }

  SUBCASE("uint16")
  {
    std::array<uint8_t, 2> dest = { {} };
    uint64_t n = 15248;

    size_t rv = h3c_varint_serialize(dest.data(), dest.size(), n);

    REQUIRE(rv == 2);
    REQUIRE(dest[0] == (0x3b | VARINT_UINT16_HEADER));
    REQUIRE(dest[1] == 0x90);

    rv = h3c_varint_parse(dest.data(), dest.size(), &n);

    REQUIRE(rv == 2);
    REQUIRE(n == 15248);
  }

  SUBCASE("uint32")
  {
    std::array<uint8_t, 4> dest = { {} };
    uint64_t n = 1073721823;

    size_t rv = h3c_varint_serialize(dest.data(), dest.size(), n);

    REQUIRE(rv == 4);
    REQUIRE(dest[0] == (0x3f | VARINT_UINT32_HEADER));
    REQUIRE(dest[1] == 0xff);
    REQUIRE(dest[2] == 0xb1);
    REQUIRE(dest[3] == 0xdf);

    rv = h3c_varint_parse(dest.data(), dest.size(), &n);

    REQUIRE(rv == 4);
    REQUIRE(n == 1073721823);
  }

  SUBCASE("uint64")
  {
    std::array<uint8_t, 8> dest = { {} };
    uint64_t n = 4611386010427387203;

    size_t rv = h3c_varint_serialize(dest.data(), dest.size(), n);

    REQUIRE(rv == 8);
    REQUIRE(dest[0] == (0x3f | VARINT_UINT64_HEADER));
    REQUIRE(dest[1] == 0xfe);
    REQUIRE(dest[2] == 0xef);
    REQUIRE(dest[3] == 0x24);
    REQUIRE(dest[4] == 0xf1);
    REQUIRE(dest[5] == 0xba);
    REQUIRE(dest[6] == 0xed);
    REQUIRE(dest[7] == 0x43);

    rv = h3c_varint_parse(dest.data(), dest.size(), &n);

    REQUIRE(rv == 8);
    REQUIRE(n == 4611386010427387203);
  }

  SUBCASE("max")
  {
    std::array<uint8_t, 8> dest = { {} };
    uint64_t n = 4611686018427387903;

    size_t rv = h3c_varint_serialize(dest.data(), dest.size(), n);

    REQUIRE(rv == 8);
    REQUIRE(dest[0] == (0x3f | VARINT_UINT64_HEADER));
    REQUIRE(dest[1] == 0xff);
    REQUIRE(dest[2] == 0xff);
    REQUIRE(dest[3] == 0xff);
    REQUIRE(dest[4] == 0xff);
    REQUIRE(dest[5] == 0xff);
    REQUIRE(dest[6] == 0xff);
    REQUIRE(dest[7] == 0xff);

    rv = h3c_varint_parse(dest.data(), dest.size(), &n);

    REQUIRE(rv == 8);
    REQUIRE(n == 4611686018427387903);
  }

  SUBCASE("overflow")
  {
    std::array<uint8_t, 8> dest = { {} };
    uint64_t n = UINT64_MAX;

    size_t rv = h3c_varint_serialize(dest.data(), dest.size(), n);
    REQUIRE(rv == 0);
  }

  SUBCASE("buffer too small")
  {
    std::array<uint8_t, 1> dest = { {} };
    uint8_t n = 169;

    // `h3c_varint_serialize` returns 0 both when the varint overflows and when
    // the varint doesn't fit in `dest`. To make sure we're testing the out of
    // bounds case, we verify that the varint doesn't overflow first.
    size_t rv = h3c_varint_size(n);
    REQUIRE(rv != 0);

    rv = h3c_varint_serialize(dest.data(), dest.size(), n);
    REQUIRE(rv == 0);
  }
}