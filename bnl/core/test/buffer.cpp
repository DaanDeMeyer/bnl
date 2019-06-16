#include <doctest/doctest.h>

#include <bnl/buffer.hpp>

using namespace bnl;

TEST_CASE("buffer")
{
  SUBCASE("unique")
  {
    std::unique_ptr<uint8_t[]> unique(new uint8_t[1000]);

    unique[30] = 10;

    buffer first(std::move(unique), 1000);
    REQUIRE(first.size() == 1000);

    buffer second = std::move(first);
    REQUIRE(second.size() == 1000);

    REQUIRE(second[30] == 10);
  }

  SUBCASE("shared")
  {
    std::shared_ptr<uint8_t> shared(new uint8_t[1000],
                                    std::default_delete<uint8_t[]>());

    shared.get()[300] = 88;

    buffer data(shared, 1000);

    REQUIRE(data.data() == shared.get());
    REQUIRE(data.size() == 1000);
    REQUIRE(shared.use_count() == 2);

    {
      data.consume(250);
      buffer slice = data.slice(500);
      data.undo(250);

      REQUIRE(slice.data() == shared.get() + 250);
      REQUIRE(slice.size() == 500);
      REQUIRE(shared.use_count() == 3);
      REQUIRE(slice[50] == 88);
    }

    REQUIRE(shared.use_count() == 2);
    REQUIRE(data[300] == 88);
  }

  SUBCASE("upgrade")
  {
    std::unique_ptr<uint8_t[]> shared(new uint8_t[1000]);

    shared[5] = 104;

    buffer first(std::move(shared), 1000);
    buffer second(first);  // NOLINT
    buffer third = second; // NOLINT

    REQUIRE(third[5] == 104);
  }

  SUBCASE("scope")
  {
    buffer data;

    {
      std::shared_ptr<uint8_t> shared(new uint8_t[1000],
                                      std::default_delete<uint8_t[]>());
      shared.get()[780] = 189;
      data = buffer(shared, 1000);
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
    std::unique_ptr<uint8_t[]> unique(new uint8_t[20]);
    unique[10] = 123;

    buffer first(std::move(unique), 20);
    buffer second = first; // NOLINT

    REQUIRE(second[10] == 123);
  }

  SUBCASE("mutable")
  {
    buffer_mut data(500);
    data[50] = 20;

    buffer second = data; // NOLINT
    REQUIRE(second[50] == 20);
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

    data.undo(1);
    REQUIRE(data[0] == 'c');
  }
}
