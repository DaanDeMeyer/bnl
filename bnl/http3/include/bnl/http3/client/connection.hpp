#pragma once

#include <bnl/http3/client/stream/control.hpp>
#include <bnl/http3/client/stream/request.hpp>
#include <bnl/http3/endpoint/generator.hpp>
#include <bnl/http3/event.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/quic/event.hpp>

#include <map>

namespace bnl {
namespace http3 {

namespace request {
using handle = endpoint::stream::request::sender::handle;
}

namespace client {

class connection;

using generator = endpoint::generator<connection>;

class BNL_HTTP3_EXPORT connection {
public:
  connection() = default;

  connection(connection &&) = default;
  connection &operator=(connection &&) = default;

  result<quic::event> send() noexcept;

  result<generator> recv(quic::event event);

  result<request::handle> request();

private:
  friend generator;

  result<event> process(uint64_t id);

  result<void> recv(quic::data data);

private:
  // Use `_t` to avoid conflict with `request` method.
  using control_t = std::pair<client::stream::control::sender,
                              client::stream::control::receiver>;
  using request_t = std::pair<client::stream::request::sender,
                              client::stream::request::receiver>;

  struct {
    settings local;
    settings peer;
  } settings_;

  control_t control_;

  std::map<uint64_t, request_t> requests_;
  uint64_t next_stream_id_ = 0;
};

}
}
}
