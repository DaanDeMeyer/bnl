#include <doctest.h>

#include <bnl/base/buffers.hpp>

using namespace bnl;

TEST_CASE("buffers")
{
  base::buffers buffers;

  REQUIRE(buffers.size() == 0);

  base::buffer first = "abc";
  base::buffer second = "fdeapdf";
  base::buffer third = "a";

  buffers.push(first);
  buffers.push(second);
  buffers.push(third);

  REQUIRE(buffers.size() == first.size() + second.size() + third.size());

  buffers.consume(5);
}
