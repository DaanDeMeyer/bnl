#pragma once

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

class BNL_HTTP3_EXPORT server
{
public:
  explicit server(const log::api* logger);

  server(server&& other) = default;
  server& operator=(server&& other) = default;

  base::result<quic::event> send() noexcept;

  std::error_code recv(quic::event event, event::handler handler);

  std::error_code header(uint64_t id, header_view header);
  std::error_code body(uint64_t id, base::buffer body);

  std::error_code start(uint64_t id) noexcept;
  std::error_code fin(uint64_t id) noexcept;

private:
  using control = std::pair<endpoint::server::control::sender,
                            endpoint::server::control::receiver>;
  using request = std::pair<endpoint::server::request::sender,
                            endpoint::server::request::receiver>;

  struct
  {
    settings local;
    settings remote;
  } settings_;

  control control_;
  std::map<uint64_t, request> requests_;

  const log::api* logger_;
};

} // namespace http3
} // namespace bnl
