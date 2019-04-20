#include <doctest.h>

#include <h3c/frame.h>

#include <array>
#include <cstring>

// We pass the storage buffer to the function because `h3c_frame_parse` does not
// copy data and header blocks from the input, but instead just stores
// where a buffer starts in the input and how large it is. If we declare the
// input buffer inside the function, the buffer pointers in the parsed frame
// would point to invalid memory when the function returns.
template <size_t N>
static h3c_frame_t serialize_and_parse(const h3c_frame_t &src,
                                       std::array<uint8_t, N> &buffer)
{
  int error = 0;
  CAPTURE(error);

  size_t bytes_written = 0;
  CAPTURE(bytes_written);

  error = h3c_frame_serialize(buffer.data(), buffer.size(), &src,
                              &bytes_written);

  REQUIRE(!error);
  REQUIRE(bytes_written == N);

  size_t bytes_read = 0;
  CAPTURE(bytes_read);

  h3c_frame_t dest;
  error = h3c_frame_parse(buffer.data(), buffer.size(), &dest, &bytes_read);

  REQUIRE(!error);
  REQUIRE(bytes_read == bytes_written);

  REQUIRE(dest.type == src.type);

  return dest;
}

TEST_CASE("frame")
{
  SUBCASE("data")
  {
    h3c_frame_t src;
    src.type = H3C_DATA;

    std::array<uint8_t, 4> payload = { 1, 2, 3, 4 };
    src.data.payload.data = payload.data();
    src.data.payload.size = payload.size();

    std::array<uint8_t, 6> buffer = { {} };
    h3c_frame_t dest = serialize_and_parse(src, buffer);

    REQUIRE(dest.data.payload.size == payload.size());
    REQUIRE(std::memcmp(dest.data.payload.data, src.data.payload.data,
                        src.data.payload.size) == 0);
  }

  SUBCASE("headers")
  {
    h3c_frame_t src;
    src.type = H3C_HEADERS;

    std::array<uint8_t, 4> header_block = { 1, 2, 3, 4 };
    src.headers.header_block.data = header_block.data();
    src.headers.header_block.size = header_block.size();

    std::array<uint8_t, 6> buffer = { {} };
    h3c_frame_t dest = serialize_and_parse(src, buffer);

    REQUIRE(dest.headers.header_block.size == src.headers.header_block.size);
    REQUIRE(std::memcmp(dest.headers.header_block.data,
                        src.headers.header_block.data,
                        dest.headers.header_block.size) == 0);
  }

  SUBCASE("priority")
  {
    h3c_frame_t src;
    src.type = H3C_PRIORITY;
    src.priority.prioritized_element_type = H3C_STREAM_CURRENT;
    src.priority.element_dependency_type = H3C_STREAM_PLACEHOLDER;
    src.priority.prioritized_element_id = 16482;     // varint size = 4
    src.priority.element_dependency_id = 1073781823; // varint size = 8
    src.priority.weight = 43;

    std::array<uint8_t, 16> buffer = { {} };
    h3c_frame_t dest = serialize_and_parse(src, buffer);

    REQUIRE(dest.priority.prioritized_element_type ==
            src.priority.prioritized_element_type);
    REQUIRE(dest.priority.element_dependency_type ==
            src.priority.element_dependency_type);
    REQUIRE(dest.priority.prioritized_element_id ==
            src.priority.prioritized_element_id);
    REQUIRE(dest.priority.weight == src.priority.weight);
  }

  SUBCASE("cancel push")
  {
    h3c_frame_t src;
    src.type = H3C_CANCEL_PUSH;
    src.cancel_push.push_id = 64; // varint size = 2

    std::array<uint8_t, 4> buffer = { {} };
    h3c_frame_t dest = serialize_and_parse(src, buffer);

    REQUIRE(dest.cancel_push.push_id == src.cancel_push.push_id);
  }

  SUBCASE("settings")
  {
    h3c_frame_t src;
    src.type = H3C_SETTINGS;
    src.settings.max_header_list_size = 63; // varint size = 1
    src.settings.num_placeholders = 16383;  // varint size = 2

    std::array<uint8_t, 7> buffer = { {} };
    h3c_frame_t dest = serialize_and_parse(src, buffer);

    REQUIRE(dest.settings.max_header_list_size ==
            src.settings.max_header_list_size);
    REQUIRE(dest.settings.num_placeholders == src.settings.num_placeholders);
  }

  SUBCASE("push promise")
  {
    h3c_frame_t src;
    src.type = H3C_PUSH_PROMISE;
    src.push_promise.push_id = 16384; // varint size = 4

    std::array<uint8_t, 7> header_block = { 1, 2, 3, 4, 5, 6, 7 };
    src.push_promise.header_block.data = header_block.data();
    src.push_promise.header_block.size = header_block.size();

    std::array<uint8_t, 13> buffer = { {} };
    h3c_frame_t dest = serialize_and_parse(src, buffer);

    REQUIRE(dest.push_promise.push_id == src.push_promise.push_id);
    REQUIRE(dest.push_promise.header_block.size ==
            src.push_promise.header_block.size);
    REQUIRE(std::memcmp(dest.push_promise.header_block.data,
                        src.push_promise.header_block.data,
                        dest.push_promise.header_block.size) == 0);
  }

  SUBCASE("goaway")
  {
    h3c_frame_t src;
    src.type = H3C_GOAWAY;
    src.goaway.stream_id = 1073741823; // varint size = 4

    std::array<uint8_t, 6> buffer = { {} };
    h3c_frame_t dest = serialize_and_parse(src, buffer);

    REQUIRE(dest.goaway.stream_id == src.goaway.stream_id);
  }

  SUBCASE("max push id")
  {
    h3c_frame_t src;
    src.type = H3C_MAX_PUSH_ID;
    src.max_push_id.push_id = 1073741824; // varint size = 8;

    std::array<uint8_t, 10> buffer = { {} };
    h3c_frame_t dest = serialize_and_parse(src, buffer);

    REQUIRE(dest.max_push_id.push_id == src.max_push_id.push_id);
  }

  SUBCASE("duplicate push")
  {
    h3c_frame_t src;
    src.type = H3C_DUPLICATE_PUSH;
    src.duplicate_push.push_id = 4611686018427387903; // varint size = 8

    std::array<uint8_t, 10> buffer = { {} };
    h3c_frame_t dest = serialize_and_parse(src, buffer);

    REQUIRE(dest.duplicate_push.push_id == src.duplicate_push.push_id);
  }

  SUBCASE("parse: frame incomplete")
  {
    h3c_frame_t src;
    src.type = H3C_DUPLICATE_PUSH;
    src.duplicate_push.push_id = 50;

    std::array<uint8_t, 3> buffer = { {} };
    size_t bytes_written = 0;
    int error = h3c_frame_serialize(buffer.data(), buffer.size(), &src,
                                    &bytes_written);

    REQUIRE(!error);
    REQUIRE(bytes_written == buffer.size());

    h3c_frame_t dest;
    size_t bytes_read = 0;
    error = h3c_frame_parse(buffer.data(), buffer.size() - 1, &dest,
                            &bytes_read);

    REQUIRE(error == H3C_FRAME_PARSE_INCOMPLETE);
    REQUIRE(bytes_read == 2); // Only frame type and length can be parsed.

    error = h3c_frame_parse(buffer.data(), buffer.size(), &dest, &bytes_read);

    REQUIRE(!error);
    REQUIRE(bytes_read == buffer.size());
  }

  SUBCASE("parse: frame malformed")
  {
    h3c_frame_t src;
    src.type = H3C_CANCEL_PUSH;
    src.cancel_push.push_id = 16384; // varint size = 4

    std::array<uint8_t, 20> buffer = { {} };
    size_t bytes_written = 0;
    int error = h3c_frame_serialize(buffer.data(), buffer.size(), &src,
                                    &bytes_written);

    REQUIRE(!error);
    REQUIRE(bytes_written == 6);

    buffer[1] = 16; // mangle the frame length

    h3c_frame_t dest;
    size_t bytes_read = 0;
    error = h3c_frame_parse(buffer.data(), buffer.size(), &dest, &bytes_read);

    REQUIRE(error == H3C_FRAME_PARSE_MALFORMED);
    REQUIRE(bytes_read == 6); // Only frame type and length can be parsed.
  }

  SUBCASE("serialize: buffer too small")
  {
    h3c_frame_t src;
    src.type = H3C_DATA;

    std::array<uint8_t, 20> payload = { {} };
    src.data.payload.data = payload.data();
    src.data.payload.size = payload.size();

    std::array<uint8_t, 10> buffer = { {} };
    size_t bytes_written = 0;
    int error = h3c_frame_serialize(buffer.data(), buffer.size(), &src,
                                    &bytes_written);

    REQUIRE(error == H3C_FRAME_SERIALIZE_BUF_TOO_SMALL);
    REQUIRE(bytes_written == 2); // Only type and length will be serialized.
  }

  SUBCASE("serialize: varint overflow")
  {
    h3c_frame_t src;
    src.type = H3C_SETTINGS;
    src.settings.max_header_list_size = 4611686018427387904; // overflows
    src.settings.num_placeholders = 15;

    std::array<uint8_t, 20> buffer = { {} };
    size_t bytes_written = 0;
    int error = h3c_frame_serialize(buffer.data(), buffer.size(), &src,
                                    &bytes_written);

    REQUIRE(error == H3C_FRAME_SERIALIZE_VARINT_OVERFLOW);
    REQUIRE(bytes_written == 0);
  }
}