#include <doctest/doctest.h>

#include <h3c/error.hpp>
#include <h3c/frame.hpp>
#include <h3c/log.hpp>

#include <array>
#include <cstring>

template <size_t N>
static h3c::frame encode_and_decode(const h3c::frame &src,
                                    const h3c::logger &logger)
{
  std::array<uint8_t, N> buffer = {};

  std::error_code error;

  size_t encoded_size = h3c::frame::encoded_size(src);
  REQUIRE(encoded_size == N);

  error = h3c::frame::encode(buffer.data(), buffer.size(), src, &encoded_size,
                             &logger);

  REQUIRE(!error);
  REQUIRE(encoded_size == N);

  h3c::frame dest = {};
  error = h3c::frame::decode(buffer.data(), buffer.size(), &dest, &encoded_size,
                             &logger);

  REQUIRE(!error);
  REQUIRE(encoded_size == N);

  REQUIRE(dest.type == src.type);

  return dest;
}

TEST_CASE("frame")
{
  h3c::logger logger;

  SUBCASE("data")
  {
    h3c::frame src = {};
    src.type = h3c::frame::type::data;
    src.data.size = 64; // frame length varint size = 2

    h3c::frame dest = encode_and_decode<3>(src, logger);

    REQUIRE(dest.data.size == src.data.size);
  }

  SUBCASE("headers")
  {
    h3c::frame src = {};
    src.type = h3c::frame::type::headers;
    src.headers.size = 16384; // frame length varint size = 4

    h3c::frame dest = encode_and_decode<5>(src, logger);

    REQUIRE(dest.headers.size == src.headers.size);
  }

  SUBCASE("priority")
  {
    h3c::frame src = {};
    src.type = h3c::frame::type::priority;
    src.priority.prioritized_element_type =
        h3c::frame::payload::priority::type::current;
    src.priority.element_dependency_type =
        h3c::frame::payload::priority::type::placeholder;
    src.priority.prioritized_element_id = 16482;     // varint size = 4
    src.priority.element_dependency_id = 1073781823; // varint size = 8
    src.priority.weight = 43;

    h3c::frame dest = encode_and_decode<16>(src, logger);

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
    h3c::frame src = {};
    src.type = h3c::frame::type::cancel_push;
    src.cancel_push.push_id = 64; // varint size = 2

    h3c::frame dest = encode_and_decode<4>(src, logger);

    REQUIRE(dest.cancel_push.push_id == src.cancel_push.push_id);
  }

  SUBCASE("settings")
  {
    h3c::frame src = {};
    src.type = h3c::frame::type::settings;
    src.settings = h3c::settings::initial();

    h3c::frame dest = encode_and_decode<17>(src, logger);

    REQUIRE(dest.settings.max_header_list_size ==
            src.settings.max_header_list_size);
    REQUIRE(dest.settings.num_placeholders == src.settings.num_placeholders);
  }

  SUBCASE("push promise")
  {
    h3c::frame src = {};
    src.type = h3c::frame::type::push_promise;
    src.push_promise.push_id = 16384; // varint size = 4
    // frame length varint size = 8
    src.push_promise.headers.size = 1073741824;

    h3c::frame dest = encode_and_decode<13>(src, logger);

    REQUIRE(dest.push_promise.push_id == src.push_promise.push_id);
    REQUIRE(dest.push_promise.headers.size == src.push_promise.headers.size);
  }

  SUBCASE("goaway")
  {
    h3c::frame src = {};
    src.type = h3c::frame::type::goaway;
    src.goaway.stream_id = 1073741823; // varint size = 4

    h3c::frame dest = encode_and_decode<6>(src, logger);

    REQUIRE(dest.goaway.stream_id == src.goaway.stream_id);
  }

  SUBCASE("max push id")
  {
    h3c::frame src = {};
    src.type = h3c::frame::type::max_push_id;
    src.max_push_id.push_id = 1073741824; // varint size = 8;

    h3c::frame dest = encode_and_decode<10>(src, logger);

    REQUIRE(dest.max_push_id.push_id == src.max_push_id.push_id);
  }

  SUBCASE("duplicate push")
  {
    h3c::frame src = {};
    src.type = h3c::frame::type::duplicate_push;
    src.duplicate_push.push_id = 4611686018427387903; // varint size = 8

    h3c::frame dest = encode_and_decode<10>(src, logger);

    REQUIRE(dest.duplicate_push.push_id == src.duplicate_push.push_id);
  }

  SUBCASE("encode: buffer too small")
  {
    h3c::frame src = {};
    src.type = h3c::frame::type::settings;
    src.settings = h3c::settings::initial();

    std::array<uint8_t, 3> buffer = {};
    size_t encoded_size = 0;
    std::error_code error = h3c::frame::encode(buffer.data(), buffer.size(),
                                               src, &encoded_size, &logger);

    REQUIRE(error == h3c::error::buffer_too_small);
    REQUIRE(encoded_size == 0);
  }

  SUBCASE("encode: varint overflow")
  {
    h3c::frame src = {};
    src.type = h3c::frame::type::data;
    src.data.size = 4611686018427387904; // overflows

    std::array<uint8_t, 20> buffer = {};
    size_t encoded_size = 0;
    std::error_code error = h3c::frame::encode(buffer.data(), buffer.size(),
                                               src, &encoded_size, &logger);

    REQUIRE(error == h3c::error::varint_overflow);
    REQUIRE(encoded_size == 0);
  }

  SUBCASE("encode: setting overflow")
  {
    h3c::frame src = {};
    src.type = h3c::frame::type::settings;
    src.settings = h3c::settings::initial();
    src.settings.qpack_max_table_capacity = 1U << 30U; // overflows

    std::array<uint8_t, 30> buffer = {};
    size_t encoded_size = 0;
    std::error_code error = h3c::frame::encode(buffer.data(), buffer.size(),
                                               src, &encoded_size, &logger);

    REQUIRE(error == h3c::error::setting_overflow);
    REQUIRE(encoded_size == 0);
  }

  SUBCASE("decode: incomplete")
  {
    h3c::frame src = {};
    src.type = h3c::frame::type::duplicate_push;
    src.duplicate_push.push_id = 50;

    std::array<uint8_t, 3> buffer = {};
    size_t encoded_size = 0;
    std::error_code error = h3c::frame::encode(buffer.data(), buffer.size(),
                                               src, &encoded_size, &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == buffer.size());

    h3c::frame dest = {};
    error = h3c::frame::decode(buffer.data(), buffer.size() - 1, &dest,
                               &encoded_size, &logger);

    REQUIRE(error == h3c::error::incomplete);
    REQUIRE(encoded_size == 0);

    error = h3c::frame::decode(buffer.data(), buffer.size(), &dest,
                               &encoded_size, &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == buffer.size());
  }

  SUBCASE("decode: frame malformed")
  {
    h3c::frame src = {};
    src.type = h3c::frame::type::cancel_push;
    src.cancel_push.push_id = 16384; // varint size = 4

    std::array<uint8_t, 20> buffer = {};
    size_t encoded_size = 0;
    std::error_code error = h3c::frame::encode(buffer.data(), buffer.size(),
                                               src, &encoded_size, &logger);

    REQUIRE(!error);
    REQUIRE(encoded_size == 6);

    buffer[1] = 16; // mangle the frame length

    h3c::frame dest = {};
    error = h3c::frame::decode(buffer.data(), buffer.size(), &dest,
                               &encoded_size, &logger);

    REQUIRE(error == h3c::error::malformed_frame);
    REQUIRE(encoded_size == 0);
  }
}