#include <doctest/doctest.h>

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
    std::array<uint8_t, H3C_VARINT_UINT8_SIZE> dest = { {} };
    uint64_t n = 0;

    size_t encoded_size = 0;
    int error = h3c_varint_encode(dest.data(), dest.size(), n, &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT8_SIZE);
    REQUIRE(dest[0] == (0x00 | VARINT_UINT8_HEADER));

    error = h3c_varint_decode(dest.data(), dest.size(), &n, &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT8_SIZE);
    REQUIRE(n == 0);
  }

  SUBCASE("uint8")
  {
    std::array<uint8_t, H3C_VARINT_UINT8_SIZE> dest = { {} };
    uint64_t n = 62;

    size_t encoded_size = 0;
    int error = h3c_varint_encode(dest.data(), dest.size(), n, &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT8_SIZE);
    REQUIRE(dest[0] == (0x3E | VARINT_UINT8_HEADER));

    error = h3c_varint_decode(dest.data(), dest.size(), &n, &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT8_SIZE);
    REQUIRE(n == 62);
  }

  SUBCASE("uint16")
  {
    std::array<uint8_t, H3C_VARINT_UINT16_SIZE> dest = { {} };
    uint64_t n = 15248;

    size_t encoded_size = 0;
    int error = h3c_varint_encode(dest.data(), dest.size(), n, &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT16_SIZE);
    REQUIRE(dest[0] == (0x3b | VARINT_UINT16_HEADER));
    REQUIRE(dest[1] == 0x90);

    error = h3c_varint_decode(dest.data(), dest.size(), &n, &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT16_SIZE);
    REQUIRE(n == 15248);
  }

  SUBCASE("uint32")
  {
    std::array<uint8_t, H3C_VARINT_UINT32_SIZE> dest = { {} };
    uint64_t n = 1073721823;

    size_t encoded_size = 0;
    int error = h3c_varint_encode(dest.data(), dest.size(), n, &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT32_SIZE);
    REQUIRE(dest[0] == (0x3f | VARINT_UINT32_HEADER));
    REQUIRE(dest[1] == 0xff);
    REQUIRE(dest[2] == 0xb1);
    REQUIRE(dest[3] == 0xdf);

    error = h3c_varint_decode(dest.data(), dest.size(), &n, &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT32_SIZE);
    REQUIRE(n == 1073721823);
  }

  SUBCASE("uint64")
  {
    std::array<uint8_t, H3C_VARINT_UINT64_SIZE> dest = { {} };
    uint64_t n = 4611386010427387203;

    size_t encoded_size = 0;
    int error = h3c_varint_encode(dest.data(), dest.size(), n,
                                        &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT64_SIZE);
    REQUIRE(dest[0] == (0x3f | VARINT_UINT64_HEADER));
    REQUIRE(dest[1] == 0xfe);
    REQUIRE(dest[2] == 0xef);
    REQUIRE(dest[3] == 0x24);
    REQUIRE(dest[4] == 0xf1);
    REQUIRE(dest[5] == 0xba);
    REQUIRE(dest[6] == 0xed);
    REQUIRE(dest[7] == 0x43);

    error = h3c_varint_decode(dest.data(), dest.size(), &n, &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT64_SIZE);
    REQUIRE(n == 4611386010427387203);
  }

  SUBCASE("max")
  {
    std::array<uint8_t, H3C_VARINT_UINT64_SIZE> dest = { {} };
    uint64_t n = 4611686018427387903;

    size_t encoded_size = 0;
    int error = h3c_varint_encode(dest.data(), dest.size(), n,
                                        &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT64_SIZE);
    REQUIRE(dest[0] == (0x3f | VARINT_UINT64_HEADER));
    REQUIRE(dest[1] == 0xff);
    REQUIRE(dest[2] == 0xff);
    REQUIRE(dest[3] == 0xff);
    REQUIRE(dest[4] == 0xff);
    REQUIRE(dest[5] == 0xff);
    REQUIRE(dest[6] == 0xff);
    REQUIRE(dest[7] == 0xff);

    error = h3c_varint_decode(dest.data(), dest.size(), &n, &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT64_SIZE);
    REQUIRE(n == 4611686018427387903);
  }

  SUBCASE("encode: varint size")
  {
    uint64_t n = 159;

    size_t encoded_size = 0;
    int error = h3c_varint_encode(nullptr, 0, n, &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT16_SIZE);
  }

  SUBCASE("encoded: fixed size")
  {
    uint64_t n = 159;

    size_t encoded_size = H3C_VARINT_UINT32_SIZE;
    int error = h3c_varint_encode(nullptr, 0, n, &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT32_SIZE);
  }

  SUBCASE("encode: overflow")
  {
    std::array<uint8_t, H3C_VARINT_UINT64_SIZE> dest = { {} };
    uint64_t n = UINT64_MAX;

    size_t encoded_size = 0;
    int error = h3c_varint_encode(dest.data(), dest.size(), n,
                                        &encoded_size);

    REQUIRE(error == H3C_ERROR_VARINT_OVERFLOW);
    REQUIRE(encoded_size == 0);
  }

  SUBCASE("encode: buffer too small")
  {
    std::array<uint8_t, H3C_VARINT_UINT8_SIZE> dest = { {} };
    uint64_t n = 169;

    size_t encoded_size = 0;
    int error = h3c_varint_encode(dest.data(), dest.size(), n,
                                        &encoded_size);

    REQUIRE(error == H3C_ERROR_BUFFER_TOO_SMALL);
    REQUIRE(encoded_size == H3C_VARINT_UINT16_SIZE);
  }

  SUBCASE("decode: incomplete")
  {
    std::array<uint8_t, H3C_VARINT_UINT16_SIZE> dest = { {} };
    uint64_t n = 169;

    size_t encoded_size = 0;
    int error = h3c_varint_encode(dest.data(), dest.size(), n,
                                        &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == H3C_VARINT_UINT16_SIZE);

    error = h3c_varint_decode(dest.data(), dest.size() - 1, &n, &encoded_size);

    REQUIRE(error == H3C_ERROR_INCOMPLETE);
    REQUIRE(encoded_size == H3C_VARINT_UINT16_SIZE);
  }
}