#include <doctest.h>

#include <bnl/base/buffer.hpp>

#include <utility>

using namespace bnl;

TEST_CASE("buffer")
{
  SUBCASE("move")
  {
    base::buffer first(1000);
    REQUIRE(first.size() == 1000);

    first[30] = 10;

    base::buffer second = std::move(first);
    REQUIRE(second.size() == 1000);

    REQUIRE(second[30] == 10);
  }

  SUBCASE("scope")
  {
    base::buffer data;

    {
      data = base::buffer(1000);
      data[780] = 189;
    }

    REQUIRE(data[780] == 189);
  }

  SUBCASE("static")
  {
    base::buffer data("abcde");
    REQUIRE(data.size() == 5);
    REQUIRE(data[4] == 'e');
  }

  SUBCASE("sso")
  {
    base::buffer first(12);
    first[10] = 123;

    base::buffer second(first.data(), first.size()); // NOLINT

    REQUIRE(second[10] == 123);
  }

  SUBCASE("position")
  {
    base::buffer data("abcdef");
    REQUIRE(data[2] == 'c');

    data.consume(2);
    REQUIRE(data[0] == 'c');

    data.consume(1);
    base::buffer second = data.slice(1);
    REQUIRE(second[0] == 'd');
  }
}
