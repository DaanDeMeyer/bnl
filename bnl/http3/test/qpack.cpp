#include <doctest/doctest.h>

#include <bnl/http3/error.hpp>

#include <bnl/http3/codec/qpack.hpp>

#include <bnl/log.hpp>

#include <algorithm>

using namespace bnl;

template <size_t N>
static void encode_and_decode(const http3::header &header,
                              http3::qpack::encoder &encoder,
                              http3::qpack::decoder &decoder)
{
  std::error_code ec;

  size_t encoded_size = encoder.encoded_size(header, ec);
  REQUIRE(!ec);
  REQUIRE(encoded_size == N);

  buffer encoded = encoder.encode(header, ec);
  REQUIRE(!ec);
  REQUIRE(encoded.size() == N);

  http3::header decoded = decoder.decode(encoded, ec);

  REQUIRE(!ec);
  REQUIRE(encoded.empty());

  REQUIRE(decoded.name == header.name);
  REQUIRE(decoded.value == header.value);
}

TEST_CASE("qpack")
{
  log::api logger;

  http3::qpack::encoder encoder(&logger);
  http3::qpack::decoder decoder(&logger);

  std::error_code ec;

  SUBCASE("indexed header field")
  {
    http3::header path = { ":path", "/" };
    encode_and_decode<3>(path, encoder, decoder);
  }

  SUBCASE("literal with name reference")
  {
    http3::header authority = { ":authority", "www.example.com" };
    encode_and_decode<16>(authority, encoder, decoder);
  }

  SUBCASE("literal without name reference")
  {
    http3::header via = { "via", "1.0 fred" };
    encode_and_decode<13>(via, encoder, decoder);
  }

  SUBCASE("encode: malformed header")
  {
    http3::header via = { "Via", "1.0.fred" };
    buffer encoded = encoder.encode(via, ec);

    REQUIRE(ec == http3::error::malformed_header);
    REQUIRE(encoded.empty());
  }

  SUBCASE("decode: incomplete")
  {
    http3::header location = { "location", "/pub/WWW/People.html" };
    buffer encoded = encoder.encode(location, ec);

    encoded = encoded.slice(10);

    http3::header decoded = decoder.decode(encoded, ec);

    REQUIRE(ec == http3::error::incomplete);
    // Prefix has been decoded so size is 2 less than before.
    REQUIRE(encoded.size() == 8);
  }

  SUBCASE("decode: qpack decompression failed (indexed header field)")
  {
    mutable_buffer data(4);
    data[0] = 0;    // prefix
    data[1] = 0;    // prefix
    data[2] = 0xff; // 0xff = indexed header field
    data[3] = 100;  // 100 = unassigned index

    buffer encoded = std::move(data);

    http3::header header = decoder.decode(encoded, ec);

    REQUIRE(ec == http3::error::qpack_decompression_failed);
    // Prefix has been decoded so size is 2 less than before.
    REQUIRE(encoded.size() == 2);
  }

  SUBCASE("decode: qpack decompression failed (literal with name reference)")
  {
    mutable_buffer data(4);
    data[0] = 0;    // prefix
    data[1] = 0;    // prefix
    data[2] = 0x5f; // 0x5f = literal with name reference
    data[3] = 100;  // 100 = unassigned index

    buffer encoded = std::move(data);

    http3::header header = decoder.decode(encoded, ec);

    REQUIRE(ec == http3::error::qpack_decompression_failed);
    // Prefix has been decoded so size is 2 less than before.
    REQUIRE(encoded.size() == 2);
  }
}
