#include <doctest/doctest.h>

#include <bnl/http3/endpoint/shared/request.hpp>

#include <bnl/log.hpp>

using namespace bnl;

TEST_CASE("handle")
{
  uint64_t id = 4;
  log::api logger;

  SUBCASE("unique")
  {
    http3::endpoint::shared::request::sender request(id, &logger);

    http3::endpoint::handle first = request.handle();

    REQUIRE(first.valid());

    http3::endpoint::handle second = request.handle();

    REQUIRE(!first.valid());
    REQUIRE(second.valid());
  }

  SUBCASE("scope")
  {
    http3::endpoint::handle handle;

    {
      http3::endpoint::shared::request::sender request(id, &logger);

      handle = request.handle();
    }

    REQUIRE(!handle.valid());
  }

  SUBCASE("move")
  {
    http3::endpoint::shared::request::sender request(id, &logger);

    http3::endpoint::handle first = request.handle();
    REQUIRE(first.valid());

    http3::endpoint::handle second = std::move(first);
    REQUIRE(second.valid());

    http3::endpoint::handle third(std::move(second));
    REQUIRE(third.valid());
  }
}
