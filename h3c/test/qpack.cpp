#include <doctest/doctest.h>

#include <h3c/error.hpp>
#include <h3c/log.hpp>
#include <h3c/qpack.hpp>

#include <array>
#include <cstring>

// clang-format off
#define MAKE_HEADER(name, value) \
  { { name, sizeof(name) - 1 }, { value, sizeof(value) - 1 } }
// clang-format on

template <size_t N>
static void encode_and_decode(const h3c::header &src,
                              h3c::qpack::encoder *encoder,
                              h3c::qpack::decoder *decoder)
{
  std::array<uint8_t, N> buffer = {};

  std::error_code error;

  size_t encoded_size = encoder->encoded_size(src);
  REQUIRE(encoded_size == N);

  error = encoder->encode(buffer.data(), buffer.size(), src, &encoded_size);

  CAPTURE(error);

  REQUIRE(!error);
  REQUIRE(encoded_size == N);

  h3c::header dest = {};
  error = decoder->decode(buffer.data(), buffer.size(), &dest, &encoded_size);

  REQUIRE(!error);
  REQUIRE(encoded_size == N);

  REQUIRE(dest.name.size == src.name.size);
  REQUIRE(dest.value.size == src.value.size);

  REQUIRE(std::memcmp(dest.name.data, src.name.data, dest.name.size) == 0);
  REQUIRE(std::memcmp(dest.value.data, src.value.data, dest.value.size) == 0);
}

TEST_CASE("qpack")
{
  h3c::logger logger;

  h3c::qpack::encoder encoder(&logger);
  h3c::qpack::decoder decoder(&logger);

  SUBCASE("indexed header field")
  {
    h3c::header path = MAKE_HEADER(":path", "/");
    encode_and_decode<1>(path, &encoder, &decoder);
  }

  SUBCASE("literal with name reference")
  {
    h3c::header authority = MAKE_HEADER(":authority", "www.example.com");
    encode_and_decode<14>(authority, &encoder, &decoder);
  }

  SUBCASE("literal without name reference")
  {
    h3c::header via = MAKE_HEADER("via", "1.0 fred");
    encode_and_decode<11>(via, &encoder, &decoder);
  }

  SUBCASE("encode: buffer too small")
  {
    h3c::header link = MAKE_HEADER("link", "</feed>; rel=\"alternate\"");

    std::array<uint8_t, 21> buffer = {};

    size_t encoded_size = encoder.encoded_size(link);
    REQUIRE(encoded_size == buffer.size() + 1);

    std::error_code error = encoder.encode(buffer.data(), buffer.size(), link,
                                           &encoded_size);

    REQUIRE(error == h3c::error::buffer_too_small);
    REQUIRE(encoded_size == 0);
  }

  SUBCASE("encode: malformed header")
  {
    h3c::header link = MAKE_HEADER("Link", "</feed>; rel=\"alternate\"");

    size_t encoded_size = 0;
    std::error_code error = encoder.encode(nullptr, 0, link, &encoded_size);

    REQUIRE(error == h3c::error::malformed_header);
    REQUIRE(encoded_size == 0);
  }

  SUBCASE("decode: incomplete")
  {
    h3c::header location = MAKE_HEADER("location", "/pub/WWW/People.html");

    std::array<uint8_t, 17> buffer = {};
    size_t encoded_size = 0;
    std::error_code error = encoder.encode(buffer.data(), buffer.size(),
                                           location, &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == 17);

    error = decoder.decode(buffer.data(), buffer.size() - 1, &location,
                           &encoded_size);

    REQUIRE(error == h3c::error::incomplete);
    REQUIRE(encoded_size == 0);
  }

  SUBCASE("decode: qpack decompression failed")
  {
    // 0xff = indexed header field, 100 = unassigned index
    std::array<uint8_t, 2> buffer = { { 0xff, 100 } };

    h3c::header header = {};
    size_t encoded_size = 0;

    h3c::qpack::decoder decoder(&logger);

    std::error_code error = decoder.decode(buffer.data(), buffer.size(),
                                           &header, &encoded_size);

    REQUIRE(error == h3c::error::qpack_decompression_failed);
    REQUIRE(encoded_size == 0);

    // 0x5f = literal with name reference, 100 = unassigned index
    buffer = { { 0x5f, 100 } };

    error = decoder.decode(buffer.data(), buffer.size(), &header,
                           &encoded_size);

    REQUIRE(error == h3c::error::qpack_decompression_failed);
    REQUIRE(encoded_size == 0);
  }
}
