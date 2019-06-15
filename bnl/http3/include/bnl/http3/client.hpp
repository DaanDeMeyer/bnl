#pragma once

#include <bnl/http3/event.hpp>
#include <bnl/http3/export.hpp>

#include <bnl/http3/endpoint/client/control.hpp>
#include <bnl/http3/endpoint/client/request.hpp>
#include <bnl/http3/endpoint/handle.hpp>

#include <bnl/quic/event.hpp>

#include <bnl/class/macro.hpp>

#include <map>

namespace bnl {

namespace log {
class api;
}

namespace http3 {

class BNL_HTTP3_EXPORT client {
public:
  explicit client(const log::api *logger);

  BNL_MOVE_ONLY(client);

  quic::data send(std::error_code &ec) noexcept;

  void recv(quic::data data, event::handler handler, std::error_code &ec);

  endpoint::handle request(std::error_code &ec);

private:
  // Use `_t` to avoid conflict with `request` method.
  using control_t = std::pair<endpoint::client::control::sender,
                              endpoint::client::control::receiver>;
  using request_t = std::pair<endpoint::client::request::sender,
                              endpoint::client::request::receiver>;

  const log::api *logger_;

  struct {
    settings local;
    settings remote;
  } settings_;

  control_t control_;

  std::map<uint64_t, request_t> requests_;
  uint64_t next_stream_id_ = 0;
};

} // namespace http3
} // namespace bnl
