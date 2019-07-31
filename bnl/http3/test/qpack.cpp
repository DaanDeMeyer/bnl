#include <doctest.h>

#include <bnl/http3/codec/qpack.hpp>

#include <algorithm>

using namespace bnl;

template<size_t N>
static void
encode_and_decode(const http3::header &header,
                  http3::qpack::encoder &encoder,
                  http3::qpack::decoder &decoder)
{
  size_t encoded_size = encoder.encoded_size(header).value();
  REQUIRE(encoded_size == N);

  base::buffer encoded = encoder.encode(header).value();
  REQUIRE(encoded.size() == N);

  http3::header decoded = decoder.decode(encoded).value();
  REQUIRE(encoded.empty());

  REQUIRE(decoded.name() == header.name());
  REQUIRE(decoded.value() == header.value());
}

TEST_CASE("qpack")
{
  http3::qpack::encoder encoder;
  http3::qpack::decoder decoder;

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
    http3::result<base::buffer> r = encoder.encode(via);
    REQUIRE(r.error() == http3::error::malformed_header);
  }

  SUBCASE("decode: incomplete")
  {
    http3::header location = { "location", "/pub/WWW/People.html" };
    base::buffer encoded = encoder.encode(location).value();

    encoded = encoded.slice(10);

    http3::result<http3::header> r = decoder.decode(encoded);

    REQUIRE(r.error() == http3::error::incomplete);
    // Prefix has been decoded so size is 2 less than before.
    REQUIRE(encoded.size() == 8);
  }

  SUBCASE("decode: qpack decompression failed (indexed header field)")
  {
    base::buffer encoded(4);
    encoded[0] = 0;    // prefix
    encoded[1] = 0;    // prefix
    encoded[2] = 0xff; // 0xff = indexed header field
    encoded[3] = 100;  // 100 = unassigned index

    http3::result<http3::header> r = decoder.decode(encoded);

    REQUIRE(r.error() == http3::error::qpack_decompression_failed);
    // Prefix has been decoded so size is 2 less than before.
    REQUIRE(encoded.size() == 2);
  }

  SUBCASE("decode: qpack decompression failed (literal with name reference)")
  {
    base::buffer encoded(4);
    encoded[0] = 0;    // prefix
    encoded[1] = 0;    // prefix
    encoded[2] = 0x5f; // 0x5f = literal with name reference
    encoded[3] = 100;  // 100 = unassigned index

    http3::result<http3::header> r = decoder.decode(encoded);

    REQUIRE(r.error() == http3::error::qpack_decompression_failed);
    // Prefix has been decoded so size is 2 less than before.
    REQUIRE(encoded.size() == 2);
  }
}
