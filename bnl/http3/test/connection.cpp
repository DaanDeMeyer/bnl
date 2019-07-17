#include <doctest/doctest.h>

#include <bnl/base/error.hpp>
#include <bnl/http3/client/connection.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/http3/server/connection.hpp>
#include <bnl/log.hpp>
#include <bnl/util/test.hpp>

#include <array>
#include <map>
#include <vector>

using namespace bnl;

struct message {
  std::vector<http3::header> headers;
  base::buffer body;
};

bool
operator==(const message &first, const message &second)
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

bool
operator!=(const message &first, const message &second)
{
  return !(first == second);
}

template<typename Handle>
static void
start(Handle &handle, const message &message)
{
  std::error_code ec;
  for (const http3::header &header : message.headers) {
    ec = handle.header(header);
    REQUIRE(!ec);
  }

  ec = handle.start();
  REQUIRE(!ec);

  ec = handle.body(message.body);
  REQUIRE(!ec);

  ec = handle.fin();
  REQUIRE(!ec);
}

template<typename Sender, typename Receiver>
static message
transfer(Sender &sender, Receiver &receiver)
{
  message decoded;

  while (true) {
    base::result<quic::event> event = sender.send();
    if (event == base::error::idle) {
      break;
    }

    REQUIRE(event);
    REQUIRE(event.value() == quic::event::type::data);

    auto handler = [&decoded](http3::event event) -> std::error_code {
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

      return {};
    };

    std::error_code ec = receiver.recv(std::move(event.value()), handler);
    REQUIRE(!ec);
  }

  return decoded;
}

TEST_CASE("endpoint")
{
  log::api logger;

  http3::client::connection client(&logger);
  http3::server::connection server(&logger);

  message msg = { { { ":method", "GET" },
                    { ":scheme", "https" },
                    { ":authority", "www.example.com" },
                    { ":path", "index.html" } },
                  { "abcde" } };

  http3::request::handle request = EXTRACT(client.request(0));
  start(request, msg);

  message decoded = transfer(client, server);
  REQUIRE(decoded == msg);

  msg = { { { ":status", "200" } }, { "qsdfg" } };

  http3::response::handle response = EXTRACT(server.response(request.id()));
  start(response, msg);

  decoded = transfer(server, client);
  REQUIRE(decoded == msg);
}
