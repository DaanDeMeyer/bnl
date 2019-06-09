#include <doctest/doctest.h>

#include <h3c/endpoint/stream/request.hpp>
#include <h3c/log.hpp>
#include <h3c/qpack.hpp>

TEST_CASE("handle")
{
  uint64_t id = 4;
  h3c::logger logger;

  SUBCASE("unique")
  {
    h3c::stream::request::encoder request(id, &logger);

    h3c::stream::request::handle first = request.handle();

    REQUIRE(first.valid());

    h3c::stream::request::handle second = request.handle();

    REQUIRE(!first.valid());
    REQUIRE(second.valid());
  }

  SUBCASE("scope")
  {
    h3c::stream::request::handle handle;

    {
      h3c::stream::request::encoder request(id, &logger);

      handle = request.handle();
    }

    REQUIRE(!handle.valid());
  }

  SUBCASE("move")
  {
    h3c::stream::request::encoder request(id, &logger);

    h3c::stream::request::handle first = request.handle();
    REQUIRE(first.valid());

    h3c::stream::request::handle second = std::move(first);
    REQUIRE(second.valid());

    h3c::stream::request::handle third(std::move(second));
    REQUIRE(third.valid());
  }
}
