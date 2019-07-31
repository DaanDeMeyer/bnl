#include <client.hpp>

#include <os/dns.hpp>
#include <os/ip/address.hpp>

#include <bnl/base/system_error.hpp>
#include <bnl/log/console.hpp>

#include <csignal>
#include <iostream>
#include <memory>

#include <sys/epoll.h>

constexpr unsigned long long operator"" _KiB(unsigned long long k) // NOLINT
{
  return k * 1024;
}

constexpr unsigned long long operator"" _MiB(unsigned long long m) // NOLINT
{
  return m * 1024 * 1024;
}

constexpr unsigned long long operator"" _GiB(unsigned long long g) // NOLINT
{
  return g * 1024 * 1024 * 1024;
}

quic::params
default_quic_params()
{
  quic::params params;

  params.max_stream_data_bidi_local = 256_KiB;
  params.max_stream_data_bidi_remote = 256_KiB;
  params.max_stream_data_uni = 256_KiB;
  params.max_data = 1_MiB;
  params.max_streams_bidi = 1;
  params.max_streams_uni = 3;
  params.idle_timeout = quic::milliseconds(30000);

  return params;
}

client::client(const ip::host &host, ip::endpoint peer)
  : socket_(peer)
  , socket_watcher_(sd_.io(socket_.fd()).assume_value())
  , retransmit_(sd_.timer().assume_value())
  , timeout_((sd_.timer().assume_value()))
  , quic_(host,
          quic::path(socket_.local(), socket_.peer()),
          default_quic_params(),
          sd_.clock())
{
  sd_.signal(SIGINT).assume_value();
  sd_.signal(SIGTERM).assume_value();

  setup();

  timeout_.update(sd_.now() + quic_.timeout());
}

client::client(client &&other) noexcept
  : socket_(std::move(other.socket_))
  , sd_(std::move(other.sd_))
  , socket_watcher_(std::move(other.socket_watcher_))
  , retransmit_(std::move(other.retransmit_))
  , timeout_(std::move(other.timeout_))
  , quic_(std::move(other.quic_))
  , http3_(std::move(other.http3_))
{
  setup();
}

void
client::setup()
{
  using namespace std::placeholders;

  socket_watcher_.on_io(std::bind(&client::io, this, _1));
  retransmit_.on_expire(std::bind(&client::retransmit, this, _1));
  timeout_.on_expire(std::bind(&client::timeout, this, _1));
}

result<http3::request::handle>
client::request()
{
  http3::result<http3::request::handle> r = http3_.request();
  if (!r) {
    return r.error();
  }

  return std::move(r).value();
}

result<void>
client::io(uint32_t events)
{
  if ((events & EPOLLERR) != 0U) {
    return error(socket_.error());
  }

  if ((events & EPOLLOUT) != 0U || (events & EPOLLHUP) != 0U) {
    BNL_TRY(send());
  }

  if ((events & EPOLLIN) != 0U || (events & EPOLLRDHUP) != 0U) {
    BNL_TRY(recv());
    BNL_TRY(send());
  }

  return base::success();
}

result<void>
client::run(handler handler)
{
  on_event_ = std::move(handler);
  return sd_.run();
}

result<void>
client::send()
{
  result<void> r = base::success();

  do {
    r = send_once();
  } while (r);

  retransmit_.update(sd_.now() + quic_.expiry());
  timeout_.update(sd_.now() + quic_.timeout());

  if (r.error() == std::errc::resource_unavailable_try_again ||
      r.error() == error::idle) {
    return base::success();
  }

  return r.error();
}

result<void>
client::send_once()
{
  {
    result<void> r = socket_.send();
    if (r) {
      return base::success();
    }

    if (r.error() != error::idle) {
      return r.error();
    }
  }

  {
    quic::result<base::buffer> r = quic_.send();
    if (r) {
      socket_.add(std::move(r).value());
      return base::success();
    }

    if (r.error() != quic::error::idle) {
      return r.error();
    }
  }

  {
    http3::result<quic::event> r = http3_.send();
    if (r) {
      quic::result<void> qr = quic_.add(std::move(r).value());
      if (qr) {
        return base::success();
      }

      return qr.error();
    }

    if (r.error() != http3::error::idle) {
      return r.error();
    }
  }

  return error::idle;
}

result<void>
client::recv()
{
  result<void> r = base::success();

  do {
    r = recv_once();
  } while (r);

  timeout_.update(sd_.now() + quic_.timeout());

  if (r.error() == std::errc::resource_unavailable_try_again) {
    return base::success();
  }

  return r.error();
}

result<void>
client::recv_once()
{
  base::buffer packet = BNL_TRY(socket_.recv());

  quic::client::generator quic = BNL_TRY(quic_.recv(std::move(packet)));

  while (quic.next()) {
    quic::event event = BNL_TRY(quic.get());

    http3::client::generator http3 = BNL_TRY(http3_.recv(std::move(event)));

    while (http3.next()) {
      http3::event event = BNL_TRY(http3.get());

      result<void> r = on_event_(std::move(event));

      if (!r && r.error() == error::finished) {
        return sd_.exit();
      }

      if (!r) {
        return sd_.exit(r.error());
      }
    }
  }

  return base::success();
}

result<void>
client::error(std::error_code ec)
{
  return ec;
}

result<void>
client::retransmit(sd::event::duration usec)
{
  (void) usec;

  BNL_LOG_I("retransmit");
  BNL_TRY(quic_.expire());

  return send();
}

result<void>
client::timeout(sd::event::duration usec)
{
  (void) usec;

  BNL_LOG_I("timeout");

  return sd_.exit(error::timeout);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"

static result<void>
run(int argc, char *argv[])
{
  if (argc < 3) {
    std::cout << "Usage: ./bnl-quic-client hostname port" << std::endl;
    return error::invalid_argument;
  }

  log::console console;
  bnl::base::logger = &console;

  ip::host host(argv[1]);
  std::vector<ip::address> resolved = BNL_TRY(os::dns::resolve(host));

  if (resolved.empty()) {
    BNL_LOG_E("Failed to resolve {}", host);
    return error::resolve;
  }

  BNL_LOG_I("Host {} resolved to the following IP addresses: ", host);

  for (const ip::address &address : resolved) {
    BNL_LOG_I("{}", address);
  }

  ip::address address = resolved[0];
  ip::port port = static_cast<uint16_t>(std::stoul(argv[2]));
  ip::endpoint peer(address, port);

  client client(host, peer);

  http3::request::handle request = BNL_TRY(client.request());

  BNL_TRY(request.header({ ":method", "GET" }));
  BNL_TRY(request.header({ ":scheme", "https" }));
  BNL_TRY(request.header({ ":authority", host.name() }));
  BNL_TRY(request.header({ ":path", "/index.html" }));

  BNL_TRY(request.start());
  BNL_TRY(request.fin());

  std::vector<http3::header> headers;
  base::buffer body;

  BNL_TRY(client.run([&](http3::event event) -> result<void> {
    switch (event) {
      case http3::event::type::settings:
        break;

      case http3::event::type::header:
        headers.emplace_back(std::move(event.header.header));
        break;

      case http3::event::type::body:
        body = base::buffer::concat(body, event.body.buffer);
        break;

      case http3::event::type::finished:
        return error::finished;
    }

    return base::success();
  }));

  for (const auto &header : headers) {
    std::cout << header << std::endl;
  }

  std::cout << std::endl;

  std::cout.write(reinterpret_cast<char *>(body.data()),
                  static_cast<std::streamsize>(body.size()));

  return base::success();
}

#pragma GCC diagnostic pop

int
main(int argc, char *argv[])
{
  result<void> r = run(argc, argv);

  if (!r) {
    return 1;
  }

  return 0;
}
