#pragma once

#include <os/socket/udp.hpp>
#include <sd/event/loop.hpp>
#include <sd/event/source.hpp>

#include <bnl/http3/client/connection.hpp>
#include <bnl/ip/endpoint.hpp>
#include <bnl/quic/client/connection.hpp>
#include <bnl/quic/clock.hpp>
#include <bnl/result.hpp>

using namespace bnl;

class client {
public:
  using handler = std::function<result<void>(http3::event)>;

  client(const ip::host &host, ip::endpoint peer, const log::api *logger);

  client(client &&other) noexcept;
  client &operator=(client &&) = delete;

  result<http3::request::handle> request();

  result<void> run(handler handler);

private:
  result<void> io(uint32_t events);

  result<void> send();
  result<void> send_once();

  result<void> recv();
  result<void> recv_once();

  result<void> error(system_code sc);
  result<void> retransmit(sd::event::duration usec);
  result<void> timeout(sd::event::duration usec);

  void setup();

private:
  os::socket::udp socket_;

  sd::event::loop sd_;
  sd::event::io socket_watcher_;
  sd::event::timer retransmit_;
  sd::event::timer timeout_;

  quic::client::connection quic_;
  http3::client::connection http3_;

  handler on_event_;

  const log::api *logger_;
};
