#include <doctest/doctest.h>

#include <bnl/base/error.hpp>
#include <bnl/http3/codec/frame.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/log.hpp>
#include <bnl/util/test.hpp>

using namespace bnl;

template<size_t N>
static http3::frame
encode_and_decode(const http3::frame &frame,
                  const http3::frame::encoder &encoder,
                  const http3::frame::decoder &decoder)
{
  size_t encoded_size = EXTRACT(encoder.encoded_size(frame));
  REQUIRE(encoded_size == N);

  base::buffer encoded = EXTRACT(encoder.encode(frame));
  REQUIRE(encoded.size() == N);

  http3::frame decoded = EXTRACT(decoder.decode(encoded));
  REQUIRE(encoded.empty());

  REQUIRE(decoded == frame);

  return decoded;
}

TEST_CASE("frame")
{
  log::api logger;

  http3::frame::encoder encoder(&logger);
  http3::frame::decoder decoder(&logger);

  SUBCASE("data")
  {
    http3::frame frame = http3::frame::payload::data{ 64 };
    http3::frame decoded = encode_and_decode<3>(frame, encoder, decoder);
    REQUIRE(decoded.data.size == frame.data.size);
  }

  SUBCASE("headers")
  {
    http3::frame frame = http3::frame::payload::headers{ 16384 };
    http3::frame decoded = encode_and_decode<5>(frame, encoder, decoder);
    REQUIRE(decoded.headers.size == frame.headers.size);
  }

  SUBCASE("priority")
  {
    http3::frame::payload::priority priority{};
    priority.prioritized_element_type =
      http3::frame::payload::priority::type::current;
    priority.element_dependency_type =
      http3::frame::payload::priority::type::placeholder;
    priority.prioritized_element_id = 16482;     // varint size = 4
    priority.element_dependency_id = 1073781823; // varint size = 8
    priority.weight = 43;

    http3::frame frame = priority;
    http3::frame decoded = encode_and_decode<16>(frame, encoder, decoder);

    REQUIRE(decoded.priority.prioritized_element_type ==
            frame.priority.prioritized_element_type);
    REQUIRE(decoded.priority.element_dependency_type ==
            frame.priority.element_dependency_type);
    REQUIRE(decoded.priority.prioritized_element_id ==
            frame.priority.prioritized_element_id);
    REQUIRE(decoded.priority.weight == frame.priority.weight);
  }

  SUBCASE("cancel push")
  {
    http3::frame frame = http3::frame::payload::cancel_push{ 64 };
    http3::frame decoded = encode_and_decode<4>(frame, encoder, decoder);
    REQUIRE(decoded.cancel_push.push_id == frame.cancel_push.push_id);
  }

  SUBCASE("settings")
  {
    http3::frame frame = http3::frame::payload::settings();
    http3::frame decoded = encode_and_decode<17>(frame, encoder, decoder);
    REQUIRE(decoded.settings.max_header_list_size ==
            frame.settings.max_header_list_size);
    REQUIRE(decoded.settings.num_placeholders ==
            frame.settings.num_placeholders);
  }

  SUBCASE("push promise")
  {
    http3::frame frame =
      http3::frame::payload::push_promise{ 16384, 1073741824 };
    http3::frame decoded = encode_and_decode<13>(frame, encoder, decoder);
    REQUIRE(decoded.push_promise.push_id == frame.push_promise.push_id);
    REQUIRE(decoded.push_promise.size == frame.push_promise.size);
  }

  SUBCASE("goaway")
  {
    http3::frame frame = http3::frame::payload::goaway{ 1073741823 };
    http3::frame decoded = encode_and_decode<6>(frame, encoder, decoder);
    REQUIRE(decoded.goaway.stream_id == frame.goaway.stream_id);
  }

  SUBCASE("max push id")
  {
    http3::frame frame = http3::frame::payload::max_push_id{ 1073741824 };
    http3::frame decoded = encode_and_decode<10>(frame, encoder, decoder);
    REQUIRE(decoded.max_push_id.push_id == frame.max_push_id.push_id);
  }

  SUBCASE("duplicate push")
  {
    http3::frame frame =
      http3::frame::payload::duplicate_push{ 4611686018427387903 };

    http3::frame decoded = encode_and_decode<10>(frame, encoder, decoder);
    REQUIRE(decoded.duplicate_push.push_id == frame.duplicate_push.push_id);
  }

  SUBCASE("encode: varint overflow")
  {
    http3::frame frame = http3::frame::payload::data{ 4611686018427387904 };

    base::result<base::buffer> result = encoder.encode(frame);
    REQUIRE(result == http3::error::varint_overflow);
  }

  SUBCASE("decode: incomplete")
  {
    http3::frame frame = http3::frame::payload::duplicate_push{ 50 };

    base::buffer encoded = EXTRACT(encoder.encode(frame));
    base::buffer incomplete = encoded.copy(encoded.size() - 1);

    base::result<http3::frame> result = decoder.decode(incomplete);

    REQUIRE(result == base::error::incomplete);
    REQUIRE(incomplete.size() == encoded.size() - 1);

    http3::frame decoded = EXTRACT(decoder.decode(encoded));

    REQUIRE(encoded.empty());
    REQUIRE(frame == decoded);
  }

  SUBCASE("decode: frame malformed")
  {
    http3::frame frame = http3::frame::payload::cancel_push{ 16384 };

    base::buffer encoded = EXTRACT(encoder.encode(frame));
    REQUIRE(encoded.size() == 6);

    // Mangle the frame length.
    const_cast<uint8_t *>(encoded.data())[1] = 16; // NOLINT

    base::result<http3::frame> result = decoder.decode(encoded);

    REQUIRE(result == http3::error::malformed_frame);
    REQUIRE(encoded.size() == 6);
  }
}
