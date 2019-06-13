#include <doctest/doctest.h>

#include <bnl/buffers.hpp>

using namespace bnl;

TEST_CASE("buffers")
{
  buffers buffers;

  REQUIRE(buffers.size() == 0);
  REQUIRE(buffers.consumed() == 0);

  buffer first = "abc";
  buffer second = "fdeapdf";
  buffer third = "a";

  buffers.push(first);
  buffers.push(second);
  buffers.push(third);

  REQUIRE(buffers.size() == first.size() + second.size() + third.size());

  buffers.consume(5);
  buffers.undo(5);
}
