#include <doctest/doctest.h>

#include <h3c/error.hpp>
#include <h3c/log.hpp>
#include <h3c/qpack.hpp>

#include <algorithm>

template <size_t N>
static void encode_and_decode(const h3c::header &header,
                              h3c::qpack::encoder &encoder,
                              h3c::qpack::decoder &decoder)
{
  std::error_code ec;

  size_t encoded_size = encoder.encoded_size(header, ec);
  REQUIRE(!ec);
  REQUIRE(encoded_size == N);

  h3c::buffer encoded = encoder.encode(header, ec);
  REQUIRE(!ec);
  REQUIRE(encoded.size() == N);

  h3c::header decoded = decoder.decode(encoded, ec);

  REQUIRE(!ec);
  REQUIRE(encoded.empty());

  REQUIRE(decoded.name.size() == header.name.size());
  REQUIRE(
      std::equal(decoded.name.data(), decoded.name.end(), header.name.data()));

  REQUIRE(decoded.value.size() == header.value.size());
  REQUIRE(std::equal(decoded.value.data(), decoded.value.end(),
                     header.value.data()));
}

TEST_CASE("qpack")
{
  h3c::logger logger;

  h3c::qpack::encoder encoder(&logger);
  h3c::qpack::decoder decoder(&logger);

  std::error_code ec;

  SUBCASE("indexed header field")
  {
    h3c::header path = { ":path", "/" };
    encode_and_decode<3>(path, encoder, decoder);
  }

  SUBCASE("literal with name reference")
  {
    h3c::header authority = { ":authority", "www.example.com" };
    encode_and_decode<16>(authority, encoder, decoder);
  }

  SUBCASE("literal without name reference")
  {
    h3c::header via = { "via", "1.0 fred" };
    encode_and_decode<13>(via, encoder, decoder);
  }

  SUBCASE("encode: malformed header")
  {
    h3c::header via = { "Via", "1.0.fred" };
    h3c::buffer encoded = encoder.encode(via, ec);

    REQUIRE(ec == h3c::error::malformed_header);
    REQUIRE(encoded.empty());
  }

  SUBCASE("decode: incomplete")
  {
    h3c::header location = { "location", "/pub/WWW/People.html" };
    h3c::buffer encoded = encoder.encode(location, ec);

    encoded = encoded.slice(10);

    h3c::header decoded = decoder.decode(encoded, ec);

    REQUIRE(ec == h3c::error::incomplete);
    // Prefix has been decoded so size is 2 less than before.
    REQUIRE(encoded.size() == 8);
  }

  SUBCASE("decode: qpack decompression failed (indexed header field)")
  {
    h3c::mutable_buffer encoded(4);
    encoded[0] = 0;    // prefix
    encoded[1] = 0;    // prefix
    encoded[2] = 0xff; // 0xff = indexed header field
    encoded[3] = 100;  // 100 = unassigned index

    h3c::header header = decoder.decode(encoded, ec);

    REQUIRE(ec == h3c::error::qpack_decompression_failed);
    // Prefix has been decoded so size is 2 less than before.
    REQUIRE(encoded.size() == 2);
  }

  SUBCASE("decode: qpack decompression failed (literal with name reference)")
  {
    h3c::mutable_buffer encoded(4);
    encoded[0] = 0;    // prefix
    encoded[1] = 0;    // prefix
    encoded[2] = 0x5f; // 0x5f = literal with name reference
    encoded[3] = 100;  // 100 = unassigned index

    h3c::header header = decoder.decode(encoded, ec);

    REQUIRE(ec == h3c::error::qpack_decompression_failed);
    // Prefix has been decoded so size is 2 less than before.
    REQUIRE(encoded.size() == 2);
  }
}
