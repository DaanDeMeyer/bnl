#pragma once

#include <bnl/http3/endpoint/generator.hpp>
#include <bnl/http3/event.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/server/stream/control.hpp>
#include <bnl/http3/server/stream/request.hpp>
#include <bnl/quic/event.hpp>

#include <map>

namespace bnl {
namespace http3 {

namespace response {
using handle = endpoint::stream::request::sender::handle;
}

namespace server {

class connection;

using generator = endpoint::generator<connection>;

class BNL_HTTP3_EXPORT connection {
public:
  connection() = default;

  connection(connection &&) = default;
  connection &operator=(connection &&) = default;

  result<quic::event> send() noexcept;

  result<generator> recv(quic::event event);

  result<response::handle> response(uint64_t id);

private:
  friend generator;

  result<void> recv(quic::data data);

  result<event> process(uint64_t id);

private:
  using control = std::pair<server::stream::control::sender,
                            server::stream::control::receiver>;
  using request = std::pair<server::stream::request::sender,
                            server::stream::request::receiver>;

  struct {
    settings local;
    settings peer;
  } settings_;

  control control_;
  std::map<uint64_t, request> requests_;
};

}
}
}
