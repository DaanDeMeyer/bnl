#pragma once

#include <bnl/base/buffer.hpp>
#include <bnl/base/result.hpp>
#include <bnl/quic/clock.hpp>
#include <bnl/quic/event.hpp>
#include <bnl/quic/export.hpp>
#include <bnl/quic/params.hpp>
#include <bnl/quic/path.hpp>

#include <chrono>
#include <random>
#include <system_error>

namespace bnl {

namespace log {
class api;
}

namespace quic {

namespace endpoint {
namespace client {
class impl;
} // namespace client
} // namespace endpoint

class BNL_QUIC_EXPORT client
{
public:
  client(path path,
         const params& params,
         clock clock,
         const log::api* logger) noexcept;

  BNL_BASE_NO_COPY(client);
  BNL_BASE_CUSTOM_MOVE(client);

  ~client();

  base::result<base::buffer> send();

  std::error_code recv(base::buffer_view data, event::handler handler);

  duration timeout() const noexcept;
  duration expiry() const noexcept;

  std::error_code expire();

  void add(quic::event event);

private:
  std::unique_ptr<endpoint::client::impl> impl_;
};

} // namespace quic
} // namespace bnl
