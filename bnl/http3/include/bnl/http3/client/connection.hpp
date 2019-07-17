#pragma once

#include <bnl/http3/client/stream/control.hpp>
#include <bnl/http3/client/stream/request.hpp>
#include <bnl/http3/event.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/quic/event.hpp>

#include <map>
#include <utility>

namespace bnl {

namespace log {
class api;
}

namespace http3 {

namespace request {
using handle = endpoint::stream::request::sender::handle;
}

namespace client {

class BNL_HTTP3_EXPORT connection {
public:
  explicit connection(const log::api *logger);

  connection(connection &&other) = default;
  connection &operator=(connection &&other) = default;

  base::result<quic::event> send() noexcept;

  std::error_code recv(quic::event event, event::handler handler);

  base::result<request::handle> request(uint64_t id);

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

  const log::api *logger_;
};

}
}
}
