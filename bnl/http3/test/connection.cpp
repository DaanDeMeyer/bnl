#include <doctest.h>

#include <bnl/http3/client/connection.hpp>
#include <bnl/http3/server/connection.hpp>

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
    http3::result<void> r = handle.header(header);
    REQUIRE(r);
  }

  http3::result<void> r = handle.start();
  REQUIRE(r);

  r = handle.body(message.body);
  REQUIRE(r);

  r = handle.fin();
  REQUIRE(r);
}

template<typename Sender, typename Receiver>
static http3::result<message>
transfer(Sender &sender, Receiver &receiver)
{
  message decoded;

  while (true) {
    http3::result<quic::event> r = sender.send();
    if (!r) {
      REQUIRE(r.error() == http3::error::idle);
      break;
    }

    auto generator = BNL_TRY(receiver.recv(std::move(r).value()));

    while (generator.next()) {
      http3::event event = BNL_TRY(generator.get());

      switch (event) {
        case http3::event::type::settings:
          break;

        case http3::event::type::header:
          decoded.headers.emplace_back(std::move(event.header.header));
          break;

        case http3::event::type::body:
          decoded.body = base::buffer::concat(decoded.body, event.body.buffer);
          break;

        case http3::event::type::finished:
          break;
      }
    }
  }

  return decoded;
}

TEST_CASE("endpoint")
{
  http3::client::connection client;
  http3::server::connection server;

  message msg = { { { ":method", "GET" },
                    { ":scheme", "https" },
                    { ":authority", "www.example.com" },
                    { ":path", "index.html" } },
                  { "abcde" } };

  http3::request::handle request = client.request().value();
  start(request, msg);

  message decoded = transfer(client, server).value();
  REQUIRE(decoded == msg);

  msg = { { { ":status", "200" } }, { "qsdfg" } };

  http3::response::handle response = server.response(request.id()).value();
  start(response, msg);

  decoded = transfer(server, client).value();
  REQUIRE(decoded == msg);
}
