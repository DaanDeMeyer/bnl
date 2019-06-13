#include <doctest/doctest.h>

#include <bnl/http3/error.hpp>

#include <bnl/http3/endpoint/client.hpp>
#include <bnl/http3/endpoint/server.hpp>

#include <bnl/log.hpp>

#include <array>
#include <map>
#include <vector>

using namespace bnl;

struct message {
  std::vector<http3::header> headers;
  buffer body;
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

static void start(http3::stream::request::handle &handle, const message &message)
{
  std::error_code ec;

  for (const http3::header &header : message.headers) {
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
    http3::transport::data data = sender.send(ec);
    if (ec == http3::error::idle) {
      break;
    }

    REQUIRE(!ec);

    auto handler = [&decoded](http3::event event, std::error_code &ec) {
      REQUIRE(!ec);

      switch (event) {
        case http3::event::type::empty:
        case http3::event::type::settings:
          break;

        case http3::event::type::header:
          decoded.headers.emplace_back(std::move(event.header));
          break;

        case http3::event::type::body:
          decoded.body = buffer::concat(decoded.body, event.body);
          break;

        case http3::event::type::error:
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

  log::api logger;

  http3::client client(&logger);
  http3::server server(&logger);

  message request = { { { ":method", "GET" },
                        { ":scheme", "https" },
                        { ":authority", "www.example.com" },
                        { ":path", "index.html" } },
                      { "abcde" } };

  http3::stream::request::handle handle = client.request(ec);
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
