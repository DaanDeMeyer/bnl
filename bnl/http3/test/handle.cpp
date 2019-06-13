#include <doctest/doctest.h>

#include <bnl/http3/endpoint/stream/request.hpp>
#include <bnl/http3/qpack.hpp>
#include <bnl/log.hpp>

using namespace bnl;

TEST_CASE("handle")
{
  uint64_t id = 4;
  log::api logger;

  SUBCASE("unique")
  {
    http3::stream::request::sender request(id, &logger);

    http3::stream::request::handle first = request.handle();

    REQUIRE(first.valid());

    http3::stream::request::handle second = request.handle();

    REQUIRE(!first.valid());
    REQUIRE(second.valid());
  }

  SUBCASE("scope")
  {
    http3::stream::request::handle handle;

    {
      http3::stream::request::sender request(id, &logger);

      handle = request.handle();
    }

    REQUIRE(!handle.valid());
  }

  SUBCASE("move")
  {
    http3::stream::request::sender request(id, &logger);

    http3::stream::request::handle first = request.handle();
    REQUIRE(first.valid());

    http3::stream::request::handle second = std::move(first);
    REQUIRE(second.valid());

    http3::stream::request::handle third(std::move(second));
    REQUIRE(third.valid());
  }
}
