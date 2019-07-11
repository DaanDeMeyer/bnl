#pragma once

#include <bnl/base/macro.hpp>
#include <bnl/http3/endpoint/client/control.hpp>
#include <bnl/http3/endpoint/client/request.hpp>
#include <bnl/http3/event.hpp>
#include <bnl/http3/export.hpp>

#include <bnl/quic/event.hpp>

#include <map>

namespace bnl {

namespace log {
class api;
}

namespace http3 {

class BNL_HTTP3_EXPORT client {
public:
  explicit client(const log::api *logger);

  BNL_BASE_MOVE_ONLY(client);

  base::result<quic::event> send() noexcept;

  std::error_code recv(quic::event event, event::handler handler);

  uint64_t request(std::error_code &ec);

  std::error_code header(uint64_t id, header_view header);
  std::error_code body(uint64_t id, base::buffer body);

  std::error_code start(uint64_t id) noexcept;
  std::error_code fin(uint64_t id) noexcept;

private:
  // Use `_t` to avoid conflict with `request` method.
  using control_t = std::pair<endpoint::client::control::sender,
                              endpoint::client::control::receiver>;
  using request_t = std::pair<endpoint::client::request::sender,
                              endpoint::client::request::receiver>;

  struct {
    settings local;
    settings remote;
  } settings_;

  control_t control_;
  std::map<uint64_t, request_t> requests_;
  uint64_t next_stream_id_ = 0;

  const log::api *logger_;
};

} // namespace http3
} // namespace bnl
