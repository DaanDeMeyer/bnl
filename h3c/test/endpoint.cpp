#include <doctest/doctest.h>

#include <h3c/endpoint/client.hpp>
#include <h3c/endpoint/server.hpp>
#include <h3c/error.hpp>
#include <h3c/log/fprintf.hpp>

#include <array>
#include <map>
#include <vector>

struct message {
  std::vector<h3c::header> headers;
  h3c::buffer body;
};

bool operator==(const message &first, const message &second)
{
  if (first.headers.size() != second.headers.size()) {
    return false;
  }

  for (size_t i = 0; i < first.headers.size(); i++) {
    if (first.headers[i] != second.headers[i]) {
      return false;
    }
  }

  return first.body == second.body;
}

bool operator!=(const message &first, const message &second)
{
  return !(first == second);
}

static void start(h3c::stream::request::handle &handle, const message &message)
{
  std::error_code ec;

  for (const h3c::header &header : message.headers) {
    handle.header(header, ec);
    REQUIRE(!ec);
  }

  handle.start(ec);
  REQUIRE(!ec);

  handle.body(message.body, ec);
  REQUIRE(!ec);

  handle.fin(ec);
  REQUIRE(!ec);
}

template <typename Sender, typename Receiver>
static message transfer(Sender &sender, Receiver &receiver)
{
  message decoded;
  std::error_code ec;

  while (true) {
    h3c::quic::data data = sender.send(ec);
    if (ec == h3c::error::idle) {
      break;
    }

    REQUIRE(!ec);

    auto handler = [&decoded](h3c::event event, std::error_code &ec) {
      REQUIRE(!ec);

      switch (event) {
        case h3c::event::type::empty:
        case h3c::event::type::settings:
          break;

        case h3c::event::type::header:
          decoded.headers.emplace_back(std::move(event.header));
          break;

        case h3c::event::type::body:
          decoded.body = h3c::buffer::concat(decoded.body, event.body);
          break;

        case h3c::event::type::error:
          REQUIRE(false);
      }
    };

    receiver.recv(std::move(data), handler, ec);

    REQUIRE(!ec);
  }

  return decoded;
}

TEST_CASE("endpoint")
{
  std::error_code ec;

  h3c::logger logger;

  h3c::client client(&logger);
  h3c::server server(&logger);

  message request = { { { ":method", "GET" },
                        { ":scheme", "https" },
                        { ":authority", "www.example.com" },
                        { ":path", "index.html" } },
                      { "abcde" } };

  h3c::stream::request::handle handle = client.request(ec);
  REQUIRE(!ec);

  start(handle, request);

  message decoded = transfer(client, server);

  REQUIRE(decoded == request);

  message response = { { { ":status", "200" } }, { "qsdfg" } };

  handle = server.response(0, ec);
  REQUIRE(!ec);

  start(handle, response);

  decoded = transfer(server, client);

  REQUIRE(decoded == response);
}
