#include <doctest/doctest.h>

#include <bnl/buffer.hpp>

using namespace bnl;

TEST_CASE("buffer")
{
  SUBCASE("unique")
  {
    buffer first(1000);
    REQUIRE(first.size() == 1000);

    first[30] = 10;

    buffer second = std::move(first);
    REQUIRE(second.size() == 1000);

    REQUIRE(second[30] == 10);
  }

  SUBCASE("upgrade")
  {
    buffer first(1000);
    first[5] = 104;

    buffer second(first);  // NOLINT
    buffer third = second; // NOLINT

    REQUIRE(third[5] == 104);
  }

  SUBCASE("scope")
  {
    buffer data;

    {
      data = buffer(1000);
      data[780] = 189;
    }

    REQUIRE(data[780] == 189);
  }

  SUBCASE("static")
  {
    buffer data("abcde");
    REQUIRE(data.size() == 5);
    REQUIRE(data[4] == 'e');
  }

  SUBCASE("sso")
  {
    buffer first(20);
    first[10] = 123;

    buffer second = first; // NOLINT

    REQUIRE(second[10] == 123);
  }

  SUBCASE("position")
  {
    buffer data("abcdef");
    REQUIRE(data[2] == 'c');

    data.consume(2);
    REQUIRE(data[0] == 'c');

    data.consume(1);
    buffer second = data.slice(1);
    REQUIRE(second[0] == 'd');
  }
}
