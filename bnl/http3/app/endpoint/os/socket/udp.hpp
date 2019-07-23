#pragma once

#include <os/fd.hpp>
#include <sd/event/loop.hpp>

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/ip/endpoint.hpp>
#include <bnl/ip/host.hpp>

using namespace bnl;

namespace bnl {
namespace log {
class api;
}
}

namespace os {
namespace socket {

class udp {
public:
  udp(ip::endpoint peer, const log::api *logger_);

  udp(udp &&) = default;
  udp &operator=(udp &&) = default;

  ip::endpoint local() const;
  ip::endpoint peer() const;

  const os::fd &fd() const noexcept;

  system_code error() const noexcept;

  result<void> send();
  result<base::buffer> recv();

  void add(base::buffer buffer);

private:
  base::buffers send_buffer_;
  os::fd socket_;

  const log::api *logger_;
};

}
}
