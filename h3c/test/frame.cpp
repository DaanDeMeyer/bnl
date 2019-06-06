#include <doctest/doctest.h>

#include <h3c/error.hpp>
#include <h3c/frame.hpp>
#include <h3c/log.hpp>
#include <h3c/log/fprintf.hpp>

template <size_t N>
static h3c::frame encode_and_decode(const h3c::frame &frame,
                                    const h3c::frame::encoder &encoder,
                                    const h3c::frame::decoder &decoder)
{
  std::error_code error;

  std::error_code ec;

  size_t encoded_size = encoder.encoded_size(frame, ec);
  REQUIRE(encoded_size == N);

  h3c::buffer encoded = encoder.encode(frame, ec);

  REQUIRE(!error);
  REQUIRE(encoded.size() == N);

  h3c::frame decoded = decoder.decode(encoded, ec);

  REQUIRE(!error);
  REQUIRE(encoded.empty());

  REQUIRE(decoded == frame);

  return decoded;
}

TEST_CASE("frame")
{
  h3c::logger logger;

  h3c::frame::encoder encoder(&logger);
  h3c::frame::decoder decoder(&logger);

  std::error_code ec;

  SUBCASE("data")
  {
    h3c::frame frame = h3c::frame::payload::data{ 64 };
    h3c::frame decoded = encode_and_decode<3>(frame, encoder, decoder);
    REQUIRE(decoded.data().size == frame.data().size);
  }

  SUBCASE("headers")
  {
    h3c::frame frame = h3c::frame::payload::headers{ 16384 };
    h3c::frame decoded = encode_and_decode<5>(frame, encoder, decoder);
    REQUIRE(decoded.headers().size == frame.headers().size);
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

    h3c::frame frame = priority;
    h3c::frame decoded = encode_and_decode<16>(frame, encoder, decoder);

    REQUIRE(decoded.priority().prioritized_element_type ==
            frame.priority().prioritized_element_type);
    REQUIRE(decoded.priority().element_dependency_type ==
            frame.priority().element_dependency_type);
    REQUIRE(decoded.priority().prioritized_element_id ==
            frame.priority().prioritized_element_id);
    REQUIRE(decoded.priority().weight == frame.priority().weight);
  }

  SUBCASE("cancel push")
  {
    h3c::frame frame = h3c::frame::payload::cancel_push{ 64 };
    h3c::frame decoded = encode_and_decode<4>(frame, encoder, decoder);
    REQUIRE(decoded.cancel_push().push_id == frame.cancel_push().push_id);
  }

  SUBCASE("settings")
  {
    h3c::frame frame = h3c::frame::payload::settings();
    h3c::frame decoded = encode_and_decode<17>(frame, encoder, decoder);
    REQUIRE(decoded.settings().max_header_list_size ==
            frame.settings().max_header_list_size);
    REQUIRE(decoded.settings().num_placeholders ==
            frame.settings().num_placeholders);
  }

  SUBCASE("push promise")
  {
    h3c::frame frame = h3c::frame::payload::push_promise{ 16384, 1073741824 };
    h3c::frame decoded = encode_and_decode<13>(frame, encoder, decoder);
    REQUIRE(decoded.push_promise().push_id == frame.push_promise().push_id);
    REQUIRE(decoded.push_promise().size == frame.push_promise().size);
  }

  SUBCASE("goaway")
  {
    h3c::frame frame = h3c::frame::payload::goaway{ 1073741823 };
    h3c::frame decoded = encode_and_decode<6>(frame, encoder, decoder);
    REQUIRE(decoded.goaway().stream_id == frame.goaway().stream_id);
  }

  SUBCASE("max push id")
  {
    h3c::frame frame = h3c::frame::payload::max_push_id{ 1073741824 };
    h3c::frame decoded = encode_and_decode<10>(frame, encoder, decoder);
    REQUIRE(decoded.max_push_id().push_id == frame.max_push_id().push_id);
  }

  SUBCASE("duplicate push")
  {
    h3c::frame frame = h3c::frame::payload::duplicate_push{
      4611686018427387903
    };

    h3c::frame decoded = encode_and_decode<10>(frame, encoder, decoder);
    REQUIRE(decoded.duplicate_push().push_id == frame.duplicate_push().push_id);
  }

  SUBCASE("encode: varint overflow")
  {
    h3c::frame frame = h3c::frame::payload::data{ 4611686018427387904 };

    h3c::buffer encoded = encoder.encode(frame, ec);

    REQUIRE(ec == h3c::error::varint_overflow);
    REQUIRE(encoded.empty());
  }

  SUBCASE("encode: setting overflow")
  {
    h3c::frame::payload::settings settings;
    settings.qpack_max_table_capacity = 1U << 30U; // overflows

    h3c::frame frame = settings;

    h3c::buffer encoded = encoder.encode(frame, ec);

    REQUIRE(ec == h3c::error::setting_overflow);
    REQUIRE(encoded.empty());
  }

  SUBCASE("decode: incomplete")
  {
    h3c::frame frame = h3c::frame::payload::duplicate_push{ 50 };

    h3c::buffer encoded = encoder.encode(frame, ec);

    REQUIRE(!ec);

    h3c::buffer slice = encoded.slice(encoded.size() - 1);
    h3c::frame decoded = decoder.decode(slice, ec);

    REQUIRE(ec == h3c::error::incomplete);
    REQUIRE(slice.size() == encoded.size() - 1);

    decoded = decoder.decode(encoded, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.empty());
  }

  SUBCASE("decode: frame malformed")
  {
    h3c::frame frame = h3c::frame::payload::cancel_push{ 16384 };

    h3c::buffer encoded = encoder.encode(frame, ec);

    REQUIRE(!ec);
    REQUIRE(encoded.size() == 6);

    // Mangle the frame length.
    const_cast<uint8_t *>(encoded.data())[1] = 16; // NOLINT

    decoder.decode(encoded, ec);

    REQUIRE(ec == h3c::error::malformed_frame);
    REQUIRE(encoded.size() == 6);
  }
}