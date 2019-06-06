#include <doctest/doctest.h>

#include <h3c/error.hpp>
#include <h3c/huffman.hpp>
#include <h3c/log.hpp>

#include <algorithm>
#include <random>
#include <string>

static std::string random_string(std::string::size_type length)
{
  static const char *chrs = "0123456789"
                            "abcdefghijklmnopqrstuvwxyz"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  static std::mt19937 rg{ std::random_device{}() };
  static std::uniform_int_distribution<std::string::size_type> pick(
      0, strlen(chrs) - 1);

  std::string s;

  s.reserve(length);

  while (length-- > 0) {
    s += chrs[pick(rg)];
  }

  return s;
}

static void encode_and_decode(const std::string &string,
                              const h3c::huffman::encoder &encoder,
                              const h3c::huffman::decoder &decoder)
{
  h3c::mutable_buffer buffer(string.size());
  std::copy_n(string.data(), string.size(), buffer.data());

  h3c::buffer encoded = encoder.encode(buffer);

  std::error_code ec;
  h3c::buffer decoded = decoder.decode(encoded, encoded.size(), ec);

  REQUIRE(!ec);
  REQUIRE(decoded.size() == string.size());
  REQUIRE(std::equal(decoded.begin(), decoded.end(), string.begin()));
}

TEST_CASE("huffman")
{
  h3c::logger logger;

  h3c::huffman::encoder encoder(&logger);
  h3c::huffman::decoder decoder(&logger);

  SUBCASE("random")
  {
    for (size_t i = 0; i < 1000; i++) {
      encode_and_decode(random_string(20), encoder, decoder);
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
