#include <doctest.h>

#include <bnl/base/error.hpp>
#include <bnl/http3/client/connection.hpp>
#include <bnl/http3/error.hpp>
#include <bnl/http3/server/connection.hpp>
#include <bnl/log.hpp>

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
  for (const http3::header &header : message.headers) {
    result<void> r = handle.header(header);
    REQUIRE(r);
  }

  result<void> r = handle.start();
  REQUIRE(r);

  r = handle.body(message.body);
  REQUIRE(r);

  r = handle.fin();
  REQUIRE(r);
}

template<typename Sender, typename Receiver>
static message
transfer(Sender &sender, Receiver &receiver)
{
  message decoded;

  while (true) {
    result<quic::event> r = sender.send();
    if (!r) {
      break;
    }

    REQUIRE(r);
    REQUIRE(r.value() == quic::event::type::data);

    auto handler = [&decoded](http3::event event) -> result<void> {
      switch (event) {
        case http3::event::type::settings:
          break;

        case http3::event::type::header:
          decoded.headers.emplace_back(std::move(event.header.header));
          break;

        case http3::event::type::body:
          decoded.body = base::buffer::concat(decoded.body, event.body.buffer);
          break;
      }

      return success();
    };

    {
      bnl::result<void> x = receiver.recv(std::move(r).value(), handler);
      REQUIRE(x);
    }
  }

  return decoded;
}

TEST_CASE("endpoint")
{
  log::api logger;

  http3::client::connection client;
  http3::server::connection server;

  message msg = { { { ":method", "GET" },
                    { ":scheme", "https" },
                    { ":authority", "www.example.com" },
                    { ":path", "index.html" } },
                  { "abcde" } };

  http3::request::handle request = client.request().value();
  start(request, msg);

  message decoded = transfer(client, server);
  REQUIRE(decoded == msg);

  msg = { { { ":status", "200" } }, { "qsdfg" } };

  http3::response::handle response = server.response(request.id()).value();
  start(response, msg);

  decoded = transfer(server, client);
  REQUIRE(decoded == msg);
}
