#include <doctest/doctest.h>

#include <h3c/qpack.h>

#include <array>
#include <cstring>

#define MAKE_HEADER(header, header_name, header_value) /* NOLINT */            \
  h3c_header_t header;                                                         \
  (header).name.data = (header_name);                                          \
  (header).name.length = sizeof(header_name) - 1;                              \
  (header).value.data = (header_value);                                        \
  (header).value.length = sizeof(header_value) - 1;                            \
  (void) 0

template <size_t N> static void encode_and_decode(const h3c_header_t &src)
{
  std::array<uint8_t, N> buffer = {};

  int error = H3C_SUCCESS;
  size_t encoded_size = 0;

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

  REQUIRE(dest.name.length == src.name.length);
  REQUIRE(dest.value.length == src.value.length);

  REQUIRE(std::memcmp(dest.name.data, src.name.data, dest.name.length) == 0);
  REQUIRE(std::memcmp(dest.value.data, src.value.data, dest.value.length) == 0);

  h3c_qpack_decode_context_destroy(&context);
}

TEST_CASE("qpack")
{
  SUBCASE("indexed header field")
  {
    MAKE_HEADER(path, ":path", "/");
    encode_and_decode<1>(path);
  }

  SUBCASE("literal with name reference")
  {
    MAKE_HEADER(authority, ":authority", "www.example.com");
    encode_and_decode<14>(authority);
  }

  SUBCASE("literal without name reference")
  {
    MAKE_HEADER(via, "via", "1.0 fred");
    encode_and_decode<11>(via);
  }

  SUBCASE("encode: header size")
  {
    MAKE_HEADER(status, ":status", "200");

    size_t encoded_size = 0;
    int error = h3c_qpack_encode(nullptr, 0, &status, &encoded_size, nullptr);

    REQUIRE(!error);
    REQUIRE(encoded_size == 1);
  }

  SUBCASE("encode: buffer too small")
  {
    MAKE_HEADER(last_modified, "link", "</feed>; rel=\"alternate\"");

    std::array<uint8_t, 21> buffer = {};
    size_t encoded_size = 0;

    int error = h3c_qpack_encode(nullptr, 0, &last_modified, &encoded_size,
                                 nullptr);

    REQUIRE(!error);
    REQUIRE(encoded_size == buffer.size() + 1);

    error = h3c_qpack_encode(buffer.data(), buffer.size(), &last_modified,
                             &encoded_size, nullptr);

    REQUIRE(error == H3C_ERROR_BUFFER_TOO_SMALL);
  }

  SUBCASE("encode: malformed header")
  {
    MAKE_HEADER(last_modified, "Link", "</feed>; rel=\"alternate\"");

    size_t encoded_size = 0;
    int error = h3c_qpack_encode(nullptr, 0, &last_modified, &encoded_size,
                                 nullptr);

    REQUIRE(error == H3C_ERROR_MALFORMED_HEADER);
  }

  SUBCASE("decode: incomplete")
  {
    MAKE_HEADER(location, "location", "/pub/WWW/People.html");

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
