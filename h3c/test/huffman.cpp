#include <doctest/doctest.h>

#include <h3c/error.hpp>
#include <h3c/huffman.hpp>
#include <h3c/log.hpp>

#include <random>

static h3c::buffer random_string(size_t length)
{
  static const uint8_t characters[] = "0123456789"
                                      "abcdefghijklmnopqrstuvwxyz"
                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  static std::mt19937 rg{ std::random_device{}() };
  static std::uniform_int_distribution<size_t> pick(0, sizeof(characters) - 1);

  h3c::mutable_buffer buffer(length);

  for (size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = characters[pick(rg)];
  }

  return std::move(buffer);
}

static void encode_and_decode(const h3c::buffer &string,
                              const h3c::huffman::encoder &encoder,
                              const h3c::huffman::decoder &decoder)
{
  h3c::buffer encoded = encoder.encode(string);

  std::error_code ec;
  h3c::buffer decoded = decoder.decode(encoded, encoded.size(), ec);

  REQUIRE(!ec);
  REQUIRE(decoded.size() == string.size());
  REQUIRE(string == decoded);
}

TEST_CASE("huffman")
{
  h3c::logger logger;

  h3c::huffman::encoder encoder(&logger);
  h3c::huffman::decoder decoder(&logger);

  SUBCASE("random")
  {
    for (size_t i = 0; i < 1000; i++) {
      h3c::buffer buffer = random_string(20);
      encode_and_decode(buffer, encoder, decoder);
    }
  }

  SUBCASE("incomplete")
  {
    h3c::buffer buffer("abcde");
    h3c::buffer encoded = encoder.encode(buffer);

    h3c::buffer slice = encoded.slice(2);

    std::error_code ec;
    h3c::buffer decoded = decoder.decode(slice, encoded.size(), ec);

    REQUIRE(ec == h3c::error::incomplete);

    decoded = decoder.decode(encoded, encoded.size(), ec);

    REQUIRE(!ec);
  }
}
