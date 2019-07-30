#include <doctest.h>

#include <bnl/base/error.hpp>
#include <bnl/http3/codec/qpack/huffman.hpp>
#include <bnl/log.hpp>

#include <random>

using namespace bnl;

static base::string
random_string(size_t length)
{
  static const char characters[] = "0123456789"
                                   "abcdefghijklmnopqrstuvwxyz"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  static std::mt19937 rg{ std::random_device{}() };
  static std::uniform_int_distribution<size_t> pick(0, sizeof(characters) - 1);

  base::string string;
  string.resize(length);

  for (char &character : string) {
    character = characters[pick(rg)];
  }

  return string;
}

static void
encode_and_decode(base::string_view string)
{
  base::buffer encoded = http3::qpack::huffman::encode(string);

  base::string decoded =
    http3::qpack::huffman::decode(encoded, encoded.size()).value();

  REQUIRE(decoded.size() == string.size());
  REQUIRE(string == decoded);
}

TEST_CASE("huffman")
{
  SUBCASE("random")
  {
    for (size_t i = 0; i < 1000; i++) {
      base::string string = random_string(20);
      encode_and_decode(string);
    }
  }

  SUBCASE("incomplete")
  {
    base::string data("abcde");
    base::buffer encoded = http3::qpack::huffman::encode(data);

    base::buffer incomplete(encoded.data(), encoded.size() - 1);

    result<base::string> r =
      http3::qpack::huffman::decode(incomplete, encoded.size());

    REQUIRE(r.error() == base::error::incomplete);
    REQUIRE(incomplete.size() == encoded.size() - 1);
  }
}
