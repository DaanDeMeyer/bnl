#include <doctest/doctest.h>

#include <h3c/qpack.h>

#include <array>
#include <cstring>

// clang-format off
#define MAKE_HEADER(name, value) \
  { { name, sizeof(name) - 1 }, { value, sizeof(value) - 1 } }
// clang-format on

template <size_t N> static void encode_and_decode(const h3c_header_t &src)
{
  std::array<uint8_t, N> buffer = {};

  int error = H3C_SUCCESS;

  size_t encoded_size = h3c_qpack_encoded_size(&src);
  REQUIRE(encoded_size == N);

  error = h3c_qpack_encode(buffer.data(), buffer.size(), &src, &encoded_size,
                           nullptr);

  CAPTURE(error);

  REQUIRE(!error);
  REQUIRE(encoded_size == N);

  h3c_qpack_decode_context_t context;
  h3c_qpack_decode_context_init(&context, nullptr);

  h3c_header_t dest;
  error = h3c_qpack_decode(&context, buffer.data(), buffer.size(), &dest,
                           &encoded_size, nullptr);

  REQUIRE(!error);
  REQUIRE(encoded_size == N);

  REQUIRE(dest.name.size == src.name.size);
  REQUIRE(dest.value.size == src.value.size);

  REQUIRE(std::memcmp(dest.name.data, src.name.data, dest.name.size) == 0);
  REQUIRE(std::memcmp(dest.value.data, src.value.data, dest.value.size) == 0);

  h3c_qpack_decode_context_destroy(&context);
}

TEST_CASE("qpack")
{
  SUBCASE("indexed header field")
  {
    h3c_header_t path = MAKE_HEADER(":path", "/");
    encode_and_decode<1>(path);
  }

  SUBCASE("literal with name reference")
  {
    h3c_header_t authority = MAKE_HEADER(":authority", "www.example.com");
    encode_and_decode<14>(authority);
  }

  SUBCASE("literal without name reference")
  {
    h3c_header_t via = MAKE_HEADER("via", "1.0 fred");
    encode_and_decode<11>(via);
  }

  SUBCASE("encode: buffer too small")
  {
    h3c_header_t link = MAKE_HEADER("link", "</feed>; rel=\"alternate\"");

    std::array<uint8_t, 21> buffer = {};

    size_t encoded_size = h3c_qpack_encoded_size(&link);
    REQUIRE(encoded_size == buffer.size() + 1);

    H3C_ERROR error = h3c_qpack_encode(buffer.data(), buffer.size(), &link,
                                       &encoded_size, nullptr);

    REQUIRE(error == H3C_ERROR_BUFFER_TOO_SMALL);
  }

  SUBCASE("encode: malformed header")
  {
    h3c_header_t link = MAKE_HEADER("Link", "</feed>; rel=\"alternate\"");

    size_t encoded_size = 0;
    int error = h3c_qpack_encode(nullptr, 0, &link, &encoded_size, nullptr);

    REQUIRE(error == H3C_ERROR_MALFORMED_HEADER);
  }

  SUBCASE("decode: incomplete")
  {
    h3c_header_t location = MAKE_HEADER("location", "/pub/WWW/People.html");

    std::array<uint8_t, 17> buffer = {};
    size_t encoded_size = 0;
    int error = h3c_qpack_encode(buffer.data(), buffer.size(), &location,
                                 &encoded_size, nullptr);

    REQUIRE(!error);
    REQUIRE(encoded_size == 17);

    h3c_qpack_decode_context_t context;
    h3c_qpack_decode_context_init(&context, nullptr);

    error = h3c_qpack_decode(&context, buffer.data(), buffer.size() - 1,
                             &location, &encoded_size, nullptr);

    REQUIRE(error == H3C_ERROR_INCOMPLETE);

    h3c_qpack_decode_context_destroy(&context);
  }

  SUBCASE("decode: qpack decompression failed")
  {
    // 0xff = indexed header field, 100 = unassigned index
    std::array<uint8_t, 2> buffer = { { 0xff, 100 } };

    h3c_header_t header;
    size_t encoded_size = 0;

    h3c_qpack_decode_context_t context;
    h3c_qpack_decode_context_init(&context, nullptr);

    int error = h3c_qpack_decode(&context, buffer.data(), buffer.size(),
                                 &header, &encoded_size, nullptr);

    REQUIRE(error == H3C_ERROR_QPACK_DECOMPRESSION_FAILED);

    // 0x5f = literal with name reference, 100 = unassigned index
    buffer = { { 0x5f, 100 } };

    error = h3c_qpack_decode(&context, buffer.data(), buffer.size(), &header,
                             &encoded_size, nullptr);

    REQUIRE(error == H3C_ERROR_QPACK_DECOMPRESSION_FAILED);

    h3c_qpack_decode_context_destroy(&context);
  }
}
