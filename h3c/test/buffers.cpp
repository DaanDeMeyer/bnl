#include <doctest/doctest.h>

#include <h3c/buffers.hpp>

TEST_CASE("buffers")
{
  h3c::buffers buffers;

  REQUIRE(buffers.size() == 0);
  REQUIRE(buffers.consumed() == 0);

  h3c::buffer first = "abc";
  h3c::buffer second = "fdeapdf";
  h3c::buffer third = "a";

  buffers.push(first);
  buffers.push(second);
  buffers.push(third);

  REQUIRE(buffers.size() == first.size() + second.size() + third.size());

  buffers.consume(5);
  buffers.undo(5);
}
