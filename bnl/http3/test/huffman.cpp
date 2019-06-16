#include <doctest/doctest.h>

#include <bnl/http3/error.hpp>

#include <bnl/http3/codec/huffman.hpp>

#include <bnl/log.hpp>

#include <random>

using namespace bnl;

static buffer random_string(size_t length)
{
  static const uint8_t characters[] = "0123456789"
                                      "abcdefghijklmnopqrstuvwxyz"
                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  static std::mt19937 rg{ std::random_device{}() };
  static std::uniform_int_distribution<size_t> pick(0, sizeof(characters) - 1);

  buffer_mut buffer(length);

  for (size_t i = 0; i < buffer.size(); i++) {
    buffer[i] = characters[pick(rg)];
  }

  return std::move(buffer);
}

static void encode_and_decode(const buffer &string,
                              const http3::huffman::encoder &encoder,
                              const http3::huffman::decoder &decoder)
{
  buffer encoded = encoder.encode(string);

  std::error_code ec;
  buffer decoded = decoder.decode(encoded, encoded.size(), ec);

  REQUIRE(!ec);
  REQUIRE(decoded.size() == string.size());
  REQUIRE(string == decoded);
}

TEST_CASE("huffman")
{
  log::api logger;

  http3::huffman::encoder encoder(&logger);
  http3::huffman::decoder decoder(&logger);

  SUBCASE("random")
  {
    for (size_t i = 0; i < 1000; i++) {
      buffer buffer = random_string(20);
      encode_and_decode(buffer, encoder, decoder);
    }
  }

  SUBCASE("incomplete")
  {
    buffer data("abcde");
    buffer encoded = encoder.encode(data);

    buffer slice = encoded.slice(2);

    std::error_code ec;
    buffer decoded = decoder.decode(slice, encoded.size(), ec);

    REQUIRE(ec == http3::error::incomplete);

    decoded = decoder.decode(encoded, encoded.size(), ec);

    REQUIRE(!ec);
  }
}
