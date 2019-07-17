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
namespace client {

class BNL_HTTP3_EXPORT connection {
public:
  explicit connection(const log::api *logger);

  connection(connection &&other) = default;
  connection &operator=(connection &&other) = default;

  base::result<quic::event> send() noexcept;

  std::error_code recv(quic::event event, event::handler handler);

  uint64_t request();

  std::error_code header(uint64_t id, header_view header);
  std::error_code body(uint64_t id, base::buffer body);

  std::error_code start(uint64_t id) noexcept;
  std::error_code fin(uint64_t id) noexcept;

private:
  // Use `_t` to avoid conflict with `request` method.
  using control_t = std::pair<client::stream::control::sender,
                              client::stream::control::receiver>;
  using request_t = std::pair<client::stream::request::sender,
                              client::stream::request::receiver>;

  struct {
    settings local;
    settings remote;
  } settings_;

  control_t control_;
  std::map<uint64_t, request_t> requests_;
  uint64_t next_stream_id_ = 0;

  const log::api *logger_;
};

}
}
}
