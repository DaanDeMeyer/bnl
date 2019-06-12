#include <doctest/doctest.h>

#include <h3c/buffer.hpp>

TEST_CASE("buffer")
{
  SUBCASE("unique")
  {
    std::unique_ptr<uint8_t[]> data(new uint8_t[1000]);

    data[30] = 10;

    h3c::buffer first(std::move(data), 1000);
    REQUIRE(first.size() == 1000);

    h3c::buffer second = std::move(first);
    REQUIRE(second.size() == 1000);

    REQUIRE(second[30] == 10);
  }

  SUBCASE("shared")
  {
    std::shared_ptr<uint8_t> data(new uint8_t[1000],
                                  std::default_delete<uint8_t[]>());

    data.get()[300] = 88;

    h3c::buffer buffer(data, 1000);

    REQUIRE(buffer.data() == data.get());
    REQUIRE(buffer.size() == 1000);
    REQUIRE(data.use_count() == 2);

    {
      buffer.consume(250);
      h3c::buffer slice = buffer.slice(500);
      buffer.undo(250);

      REQUIRE(slice.data() == data.get() + 250);
      REQUIRE(slice.size() == 500);
      REQUIRE(data.use_count() == 3);
      REQUIRE(slice[50] == 88);
    }

    REQUIRE(data.use_count() == 2);
    REQUIRE(buffer[300] == 88);
  }

  SUBCASE("upgrade")
  {
    std::unique_ptr<uint8_t[]> data(new uint8_t[1000]);

    data[5] = 104;

    h3c::buffer first(std::move(data), 1000);
    h3c::buffer second(first);  // NOLINT
    h3c::buffer third = second; // NOLINT

    REQUIRE(third[5] == 104);
  }

  SUBCASE("scope")
  {
    h3c::buffer buffer;

    {
      std::shared_ptr<uint8_t> data(new uint8_t[1000],
                                    std::default_delete<uint8_t[]>());
      data.get()[780] = 189;
      buffer = h3c::buffer(data, 1000);
    }

    REQUIRE(buffer[780] == 189);
  }

  SUBCASE("static")
  {
    h3c::buffer buffer("abcde");
    REQUIRE(buffer.size() == 5);
    REQUIRE(buffer[4] == 'e');
  }

  SUBCASE("sso")
  {
    std::unique_ptr<uint8_t[]> data(new uint8_t[20]);
    data[10] = 123;

    h3c::buffer first(std::move(data), 20);
    h3c::buffer second = first; // NOLINT

    REQUIRE(second[10] == 123);
  }

  SUBCASE("mutable")
  {
    h3c::mutable_buffer buffer(500);
    buffer[50] = 20;

    h3c::buffer second = buffer; // NOLINT
    REQUIRE(second[50] == 20);
  }

  SUBCASE("position")
  {
    h3c::buffer buffer("abcdef");
    REQUIRE(buffer[2] == 'c');

    buffer.consume(2);
    REQUIRE(buffer[0] == 'c');

    buffer.consume(1);
    h3c::buffer second = buffer.slice(1);
    REQUIRE(second[0] == 'd');

    buffer.undo(1);
    REQUIRE(buffer[0] == 'c');
  }
}
