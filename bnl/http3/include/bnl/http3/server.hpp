#pragma once

#include <bnl/base/macro.hpp>
#include <bnl/base/nothing.hpp>
#include <bnl/http3/endpoint/server/control.hpp>
#include <bnl/http3/endpoint/server/request.hpp>
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

class BNL_HTTP3_EXPORT server {
public:
  explicit server(const log::api *logger);

  BNL_BASE_MOVE_ONLY(server);

  quic::event send(std::error_code &ec) noexcept;

  base::nothing recv(quic::event event,
                     event::handler handler,
                     std::error_code &ec);

  base::nothing header(uint64_t id, header_view header, std::error_code &ec);
  base::nothing body(uint64_t id, base::buffer body, std::error_code &ec);

  base::nothing start(uint64_t id, std::error_code &ec) noexcept;
  base::nothing fin(uint64_t id, std::error_code &ec) noexcept;

private:
  using control = std::pair<endpoint::server::control::sender,
                            endpoint::server::control::receiver>;
  using request = std::pair<endpoint::server::request::sender,
                            endpoint::server::request::receiver>;

  const log::api *logger_;

  struct {
    settings local;
    settings remote;
  } settings_;

  control control_;

  std::map<uint64_t, request> requests_;
  base::buffer buffered_;
};

} // namespace http3
} // namespace bnl
