#include <doctest/doctest.h>

#include <bnl/http3/error.hpp>

#include <bnl/http3/client.hpp>
#include <bnl/http3/server.hpp>

#include <bnl/base/error.hpp>
#include <bnl/log.hpp>

#include <array>
#include <map>
#include <vector>

using namespace bnl;

struct message {
  std::vector<http3::header> headers;
  base::buffer body;
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

template <typename Endpoint>
static void start(Endpoint &endpoint, uint64_t id, const message &message)
{
  std::error_code ec;

  for (const http3::header &header : message.headers) {
    endpoint.header(id, header, ec);
    REQUIRE(!ec);
  }

  endpoint.start(id, ec);
  REQUIRE(!ec);

  endpoint.body(id, message.body, ec);
  REQUIRE(!ec);

  endpoint.fin(id, ec);
  REQUIRE(!ec);
}

template <typename Sender, typename Receiver>
static message transfer(Sender &sender, Receiver &receiver)
{
  message decoded;
  std::error_code ec;

  while (true) {
    quic::event event = sender.send(ec);
    if (ec == base::error::idle) {
      break;
    }

    REQUIRE(event == quic::event::type::data);
    REQUIRE(!ec);

    auto handler = [&decoded](http3::event event, std::error_code &ec) {
      REQUIRE(!ec);

      switch (event) {
        case http3::event::type::settings:
          break;

        case http3::event::type::header:
          decoded.headers.emplace_back(std::move(event.header));
          break;

        case http3::event::type::body:
          decoded.body = base::buffer::concat(decoded.body, event.body);
          break;

        case http3::event::type::error:
          REQUIRE(false);
      }
    };

    receiver.recv(std::move(event), handler, ec);

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

  uint64_t id = client.request(ec);
  REQUIRE(!ec);

  start(client, id, request);

  message decoded = transfer(client, server);

  REQUIRE(decoded == request);

  message response = { { { ":status", "200" } }, { "qsdfg" } };

  start(server, id, response);

  decoded = transfer(server, client);

  REQUIRE(decoded == response);
}
