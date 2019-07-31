#pragma once

#include <os/fd.hpp>
#include <os/result.hpp>
#include <sd/event/loop.hpp>

#include <bnl/base/buffer.hpp>
#include <bnl/base/buffers.hpp>
#include <bnl/ip/endpoint.hpp>
#include <bnl/ip/host.hpp>

using namespace bnl;

namespace os {
namespace socket {

class udp {
public:
  udp(ip::endpoint peer);

  udp(udp &&) = default;
  udp &operator=(udp &&) = default;

  ip::endpoint local() const;
  ip::endpoint peer() const;

  const os::fd &fd() const noexcept;

  std::error_code error() const noexcept;

  result<void> send();
  result<base::buffer> recv();

  void add(base::buffer buffer);

private:
  base::buffers send_buffer_;
  os::fd socket_;
};

}
}
