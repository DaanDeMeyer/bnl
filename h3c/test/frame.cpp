#include <doctest.h>

#include <h3c/frame.h>

#include <array>
#include <cstring>

template <size_t N>
static h3c_frame_t serialize_and_parse(const h3c_frame_t &src)
{
  std::array<uint8_t, N> buffer = { {} };

  H3C_ERROR error = H3C_SUCCESS;
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
    src.type = H3C_FRAME_DATA;
    src.data.payload.size = 64; // frame length varint size = 2

    h3c_frame_t dest = serialize_and_parse<3>(src);

    REQUIRE(dest.data.payload.size == src.data.payload.size);
  }

  SUBCASE("headers")
  {
    h3c_frame_t src;
    src.type = H3C_FRAME_HEADERS;
    src.headers.header_block.size = 16384; // frame length varint size = 4

    h3c_frame_t dest = serialize_and_parse<5>(src);

    REQUIRE(dest.headers.header_block.size == src.headers.header_block.size);
  }

  SUBCASE("priority")
  {
    h3c_frame_t src;
    src.type = H3C_FRAME_PRIORITY;
    src.priority.prioritized_element_type = H3C_FRAME_PRIORITY_CURRENT;
    src.priority.element_dependency_type = H3C_FRAME_PRIORITY_PLACEHOLDER;
    src.priority.prioritized_element_id = 16482;     // varint size = 4
    src.priority.element_dependency_id = 1073781823; // varint size = 8
    src.priority.weight = 43;

    h3c_frame_t dest = serialize_and_parse<16>(src);

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
    src.type = H3C_FRAME_CANCEL_PUSH;
    src.cancel_push.push_id = 64; // varint size = 2

    h3c_frame_t dest = serialize_and_parse<4>(src);

    REQUIRE(dest.cancel_push.push_id == src.cancel_push.push_id);
  }

  SUBCASE("settings")
  {
    h3c_frame_t src;
    src.type = H3C_FRAME_SETTINGS;
    src.settings = h3c_frame_settings_default;

    h3c_frame_t dest = serialize_and_parse<17>(src);

    REQUIRE(dest.settings.max_header_list_size ==
            src.settings.max_header_list_size);
    REQUIRE(dest.settings.num_placeholders == src.settings.num_placeholders);
  }

  SUBCASE("push promise")
  {
    h3c_frame_t src;
    src.type = H3C_FRAME_PUSH_PROMISE;
    src.push_promise.push_id = 16384; // varint size = 4
    // frame length varint size = 8
    src.push_promise.header_block.size = 1073741824;

    h3c_frame_t dest = serialize_and_parse<13>(src);

    REQUIRE(dest.push_promise.push_id == src.push_promise.push_id);
    REQUIRE(dest.push_promise.header_block.size ==
            src.push_promise.header_block.size);
  }

  SUBCASE("goaway")
  {
    h3c_frame_t src;
    src.type = H3C_FRAME_GOAWAY;
    src.goaway.stream_id = 1073741823; // varint size = 4

    h3c_frame_t dest = serialize_and_parse<6>(src);

    REQUIRE(dest.goaway.stream_id == src.goaway.stream_id);
  }

  SUBCASE("max push id")
  {
    h3c_frame_t src;
    src.type = H3C_FRAME_MAX_PUSH_ID;
    src.max_push_id.push_id = 1073741824; // varint size = 8;

    h3c_frame_t dest = serialize_and_parse<10>(src);

    REQUIRE(dest.max_push_id.push_id == src.max_push_id.push_id);
  }

  SUBCASE("duplicate push")
  {
    h3c_frame_t src;
    src.type = H3C_FRAME_DUPLICATE_PUSH;
    src.duplicate_push.push_id = 4611686018427387903; // varint size = 8

    h3c_frame_t dest = serialize_and_parse<10>(src);

    REQUIRE(dest.duplicate_push.push_id == src.duplicate_push.push_id);
  }

  SUBCASE("serialize: buffer too small")
  {
    h3c_frame_t src;
    src.type = H3C_FRAME_SETTINGS;
    src.settings = h3c_frame_settings_default;

    std::array<uint8_t, 3> buffer = { {} };
    size_t bytes_written = 0;
    H3C_ERROR error = h3c_frame_serialize(buffer.data(), buffer.size(), &src,
                                          &bytes_written);

    REQUIRE(error == H3C_ERROR_BUF_TOO_SMALL);
    // Only type and length and the first setting id will be serialized.
    REQUIRE(bytes_written == buffer.size());
  }

  SUBCASE("serialize: varint overflow")
  {
    h3c_frame_t src;
    src.type = H3C_FRAME_DATA;
    src.data.payload.size = 4611686018427387904; // overflows

    std::array<uint8_t, 20> buffer = { {} };
    size_t bytes_written = 0;
    H3C_ERROR error = h3c_frame_serialize(buffer.data(), buffer.size(), &src,
                                          &bytes_written);

    REQUIRE(error == H3C_ERROR_VARINT_OVERFLOW);
    REQUIRE(bytes_written == 0);
  }

  SUBCASE("serialize: setting overflow")
  {
    h3c_frame_t src;
    src.type = H3C_FRAME_SETTINGS;
    src.settings = h3c_frame_settings_default;
    src.settings.qpack_max_table_capacity = 1U << 30; // overflows

    std::array<uint8_t, 30> buffer = { {} };
    size_t bytes_written = 0;
    H3C_ERROR error = h3c_frame_serialize(buffer.data(), buffer.size(), &src,
                                          &bytes_written);

    REQUIRE(error == H3C_ERROR_SETTING_OVERFLOW);
    REQUIRE(bytes_written == 0);
  }

  SUBCASE("parse: frame incomplete")
  {
    h3c_frame_t src;
    src.type = H3C_FRAME_DUPLICATE_PUSH;
    src.duplicate_push.push_id = 50;

    std::array<uint8_t, 3> buffer = { {} };
    size_t bytes_written = 0;
    H3C_ERROR error = h3c_frame_serialize(buffer.data(), buffer.size(), &src,
                                          &bytes_written);

    REQUIRE(!error);
    REQUIRE(bytes_written == buffer.size());

    h3c_frame_t dest;
    size_t bytes_read = 0;
    error = h3c_frame_parse(buffer.data(), buffer.size() - 1, &dest,
                            &bytes_read);

    REQUIRE(error == H3C_ERROR_FRAME_INCOMPLETE);
    REQUIRE(bytes_read == 2); // Only frame type and length can be parsed.

    error = h3c_frame_parse(buffer.data(), buffer.size(), &dest, &bytes_read);

    REQUIRE(!error);
    REQUIRE(bytes_read == buffer.size());
  }

  SUBCASE("parse: frame malformed")
  {
    h3c_frame_t src;
    src.type = H3C_FRAME_CANCEL_PUSH;
    src.cancel_push.push_id = 16384; // varint size = 4

    std::array<uint8_t, 20> buffer = { {} };
    size_t bytes_written = 0;
    H3C_ERROR error = h3c_frame_serialize(buffer.data(), buffer.size(), &src,
                                          &bytes_written);

    REQUIRE(!error);
    REQUIRE(bytes_written == 6);

    buffer[1] = 16; // mangle the frame length

    h3c_frame_t dest;
    size_t bytes_read = 0;
    error = h3c_frame_parse(buffer.data(), buffer.size(), &dest, &bytes_read);

    REQUIRE(error == H3C_ERROR_FRAME_MALFORMED);
    REQUIRE(bytes_read == 6); // Only frame type and length can be parsed.
  }
}