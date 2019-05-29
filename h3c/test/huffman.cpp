#include <doctest/doctest.h>

#include <h3c/error.hpp>
#include <h3c/huffman.hpp>
#include <h3c/log.hpp>

#include <cstdint>
#include <iostream>
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
                              const h3c::logger &logger)
{
  size_t encoded_size = h3c::huffman::encoded_size(string.data(),
                                                   string.size());

  std::vector<uint8_t> buffer(encoded_size);

  std::error_code error = h3c::huffman::encode(buffer.data(), buffer.size(),
                                               string.data(), string.size(),
                                               &logger);

  CAPTURE(error);

  REQUIRE(!error);

  std::vector<char> decode(string.size() + 20);

  size_t string_size = decode.size();
  error = h3c::huffman::decode(buffer.data(), encoded_size, decode.data(),
                               &string_size, &logger);

  CAPTURE(string);

  std::string decoded(decode.data(), string_size);

  CAPTURE(decoded);

  REQUIRE(!error);
  REQUIRE(decoded == string);
}

TEST_CASE("huffman")
{
  h3c::logger logger;
  encode_and_decode(random_string(20), logger);
}
