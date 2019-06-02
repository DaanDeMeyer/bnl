#include <doctest/doctest.h>

#include <h3c/error.hpp>
#include <h3c/frame.hpp>
#include <h3c/log.hpp>

#include <array>
#include <cstring>

template <size_t N>
static h3c::frame encode_and_decode(const h3c::frame &src,
                                    h3c::frame::encoder *encoder,
                                    h3c::frame::decoder *decoder)
{
  std::array<uint8_t, N> buffer = {};

  std::error_code error;

  size_t encoded_size = encoder->encoded_size(src);
  REQUIRE(encoded_size == N);

  error = encoder->encode(buffer.data(), buffer.size(), src, &encoded_size);

  REQUIRE(!error);
  REQUIRE(encoded_size == N);

  h3c::frame dest;
  error = decoder->decode(buffer.data(), buffer.size(), &dest, &encoded_size);

  REQUIRE(!error);
  REQUIRE(encoded_size == N);

  REQUIRE(dest == src);

  return dest;
}

TEST_CASE("frame")
{
  h3c::logger logger;

  h3c::frame::encoder encoder(&logger);
  h3c::frame::decoder decoder(&logger);

  SUBCASE("data")
  {
    h3c::frame src = h3c::frame::payload::data{ 64 };
    h3c::frame dest = encode_and_decode<3>(src, &encoder, &decoder);
    REQUIRE(dest.data().size == src.data().size);
  }

  SUBCASE("headers")
  {
    h3c::frame src = h3c::frame::payload::headers{ 16384 };
    h3c::frame dest = encode_and_decode<5>(src, &encoder, &decoder);
    REQUIRE(dest.headers().size == src.headers().size);
  }

  SUBCASE("priority")
  {
    h3c::frame::payload::priority priority{};
    priority.prioritized_element_type =
        h3c::frame::payload::priority::type::current;
    priority.element_dependency_type =
        h3c::frame::payload::priority::type::placeholder;
    priority.prioritized_element_id = 16482;     // varint size = 4
    priority.element_dependency_id = 1073781823; // varint size = 8
    priority.weight = 43;

    h3c::frame src = priority;
    h3c::frame dest = encode_and_decode<16>(src, &encoder, &decoder);

    REQUIRE(dest.priority().prioritized_element_type ==
            src.priority().prioritized_element_type);
    REQUIRE(dest.priority().element_dependency_type ==
            src.priority().element_dependency_type);
    REQUIRE(dest.priority().prioritized_element_id ==
            src.priority().prioritized_element_id);
    REQUIRE(dest.priority().weight == src.priority().weight);
  }

  SUBCASE("cancel push")
  {
    h3c::frame src = h3c::frame::payload::cancel_push{ 64 };
    h3c::frame dest = encode_and_decode<4>(src, &encoder, &decoder);
    REQUIRE(dest.cancel_push().push_id == src.cancel_push().push_id);
  }

  SUBCASE("settings")
  {
    h3c::frame src = h3c::frame::payload::settings();
    h3c::frame dest = encode_and_decode<17>(src, &encoder, &decoder);
    REQUIRE(dest.settings().max_header_list_size ==
            src.settings().max_header_list_size);
    REQUIRE(dest.settings().num_placeholders ==
            src.settings().num_placeholders);
  }

  SUBCASE("push promise")
  {
    h3c::frame src = h3c::frame::payload::push_promise{ 16384, 1073741824 };
    h3c::frame dest = encode_and_decode<13>(src, &encoder, &decoder);
    REQUIRE(dest.push_promise().push_id == src.push_promise().push_id);
    REQUIRE(dest.push_promise().size == src.push_promise().size);
  }

  SUBCASE("goaway")
  {
    h3c::frame src = h3c::frame::payload::goaway{ 1073741823 };
    h3c::frame dest = encode_and_decode<6>(src, &encoder, &decoder);
    REQUIRE(dest.goaway().stream_id == src.goaway().stream_id);
  }

  SUBCASE("max push id")
  {
    h3c::frame src = h3c::frame::payload::max_push_id{ 1073741824 };
    h3c::frame dest = encode_and_decode<10>(src, &encoder, &decoder);
    REQUIRE(dest.max_push_id().push_id == src.max_push_id().push_id);
  }

  SUBCASE("duplicate push")
  {
    h3c::frame src = h3c::frame::payload::duplicate_push{ 4611686018427387903 };
    h3c::frame dest = encode_and_decode<10>(src, &encoder, &decoder);
    REQUIRE(dest.duplicate_push().push_id == src.duplicate_push().push_id);
  }

  SUBCASE("encode: buffer too small")
  {
    h3c::frame src = h3c::frame::payload::settings{};

    std::array<uint8_t, 3> buffer = {};
    size_t encoded_size = 0;
    std::error_code error = encoder.encode(buffer.data(), buffer.size(), src,
                                           &encoded_size);

    REQUIRE(error == h3c::error::buffer_too_small);
    REQUIRE(encoded_size == 0);
  }

  SUBCASE("encode: varint overflow")
  {
    h3c::frame src = h3c::frame::payload::data{ 4611686018427387904 };

    std::array<uint8_t, 20> buffer = {};
    size_t encoded_size = 0;
    std::error_code error = encoder.encode(buffer.data(), buffer.size(), src,
                                           &encoded_size);

    REQUIRE(error == h3c::error::varint_overflow);
    REQUIRE(encoded_size == 0);
  }

  SUBCASE("encode: setting overflow")
  {
    h3c::frame::payload::settings settings;
    settings.qpack_max_table_capacity = 1U << 30U; // overflows

    h3c::frame src = settings;

    std::array<uint8_t, 30> buffer = {};
    size_t encoded_size = 0;
    std::error_code error = encoder.encode(buffer.data(), buffer.size(), src,
                                           &encoded_size);

    REQUIRE(error == h3c::error::setting_overflow);
    REQUIRE(encoded_size == 0);
  }

  SUBCASE("decode: incomplete")
  {
    h3c::frame src = h3c::frame::payload::duplicate_push{ 50 };

    std::array<uint8_t, 3> buffer = {};
    size_t encoded_size = 0;
    std::error_code error = encoder.encode(buffer.data(), buffer.size(), src,
                                           &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == buffer.size());

    h3c::frame dest;
    error = decoder.decode(buffer.data(), buffer.size() - 1, &dest,
                           &encoded_size);

    REQUIRE(error == h3c::error::incomplete);
    REQUIRE(encoded_size == 0);

    error = decoder.decode(buffer.data(), buffer.size(), &dest, &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == buffer.size());
  }

  SUBCASE("decode: frame malformed")
  {
    h3c::frame src = h3c::frame::payload::cancel_push{ 16384 };

    std::array<uint8_t, 20> buffer = {};
    size_t encoded_size = 0;
    std::error_code error = encoder.encode(buffer.data(), buffer.size(), src,
                                           &encoded_size);

    REQUIRE(!error);
    REQUIRE(encoded_size == 6);

    buffer[1] = 16; // mangle the frame length

    h3c::frame dest;
    error = decoder.decode(buffer.data(), buffer.size(), &dest, &encoded_size);

    REQUIRE(error == h3c::error::malformed_frame);
    REQUIRE(encoded_size == 0);
  }
}