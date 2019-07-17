#pragma once

#include <bnl/http3/event.hpp>
#include <bnl/http3/export.hpp>
#include <bnl/http3/server/stream/control.hpp>
#include <bnl/http3/server/stream/request.hpp>
#include <bnl/quic/event.hpp>

#include <map>
#include <utility>

namespace bnl {

namespace log {
class api;
}

namespace http3 {
namespace server {

class BNL_HTTP3_EXPORT connection {
public:
  explicit connection(const log::api *logger);

  connection(connection &&other) = default;
  connection &operator=(connection &&other) = default;

  base::result<quic::event> send() noexcept;

  std::error_code recv(quic::event event, event::handler handler);

  std::error_code header(uint64_t id, header_view header);
  std::error_code body(uint64_t id, base::buffer body);

  std::error_code start(uint64_t id) noexcept;
  std::error_code fin(uint64_t id) noexcept;

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

  const log::api *logger_;
};

}
}
}
