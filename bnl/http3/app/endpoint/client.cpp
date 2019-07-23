#include <client.hpp>

#include <os/dns/client.hpp>
#include <os/ip/address.hpp>

#include <bnl/base/error.hpp>
#include <bnl/log/console.hpp>
#include <bnl/util/error.hpp>

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

client::client(const ip::host &host, ip::endpoint peer, const log::api *logger)
  : socket_(peer, logger)
  , sd_(logger)
  , socket_watcher_(sd_.io(socket_.fd()).assume_value())
  , retransmit_(sd_.timer().assume_value())
  , timeout_((sd_.timer().assume_value()))
  , quic_(host,
          quic::path(socket_.local(), socket_.peer()),
          default_quic_params(),
          sd_.clock(),
          logger)
  , http3_(logger)
  , logger_(logger)
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
  , logger_(other.logger_)
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
  return http3_.request();
}

result<void>
client::io(uint32_t events)
{
  if ((events & EPOLLERR) != 0U) {
    return error(socket_.error());
  }

  if ((events & EPOLLOUT) != 0U || (events & EPOLLHUP) != 0U) {
    TRY(send());
  }

  if ((events & EPOLLIN) != 0U || (events & EPOLLRDHUP) != 0U) {
    TRY(recv());
    TRY(send());
  }

  return success();
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
  result<void> r = success();

  do {
    r = send_once();
  } while (r);

  retransmit_.update(sd_.now() + quic_.expiry());
  timeout_.update(sd_.now() + quic_.timeout());

  if (r.error() == errc::resource_unavailable_try_again ||
      r.error() == base::error::idle) {
    return success();
  }

  return std::move(r).error();
}

result<void>
client::send_once()
{
  {
    result<void> r = socket_.send();
    if (r) {
      return success();
    }

    if (r.error() != base::error::idle) {
      return std::move(r).error();
    }
  }

  {
    result<base::buffer> r = quic_.send();
    if (r) {
      socket_.add(std::move(r).value());
      return success();
    }

    if (r.error() != base::error::idle) {
      return std::move(r).error();
    }
  }

  {
    result<quic::event> r = http3_.send();
    if (r) {
      return quic_.add(std::move(r).value());
    }

    if (r.error() != base::error::idle) {
      return std::move(r).error();
    }
  }

  return base::error::idle;
}

result<void>
client::recv()
{
  result<void> r = success();

  do {
    r = recv_once();
  } while (r);

  timeout_.update(sd_.now() + quic_.timeout());

  if (r.error() == errc::resource_unavailable_try_again) {
    return success();
  }

  return std::move(r).error();
}

result<void>
client::recv_once()
{
  base::buffer packet = TRY(socket_.recv());

  auto on_quic_event = [this](quic::event event) -> result<void> {
    result<void> r = http3_.recv(std::move(event), on_event_);

    if (!r && r.error() == base::error::finished) {
      return sd_.exit();
    }

    return r;
  };

  TRY(quic_.recv(std::move(packet), std::move(on_quic_event)));

  return success();
}

result<void>
client::error(system_code sc)
{
  LOG_E("{}", sc.message());
  return failure(std::move(sc));
}

result<void>
client::retransmit(sd::event::duration usec)
{
  (void) usec;

  LOG_I("retransmit");
  TRY(quic_.expire());

  return send();
}

result<void>
client::timeout(sd::event::duration usec)
{
  (void) usec;

  LOG_I("timeout");

  return sd_.exit(errc::timed_out);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"

static result<void>
run(int argc, char *argv[])
{
  if (argc < 3) {
    std::cout << "Usage: ./bnl-quic-client hostname port" << std::endl;
    return errc::invalid_argument;
  }

  log::console logger;
  os::dns::client dns(&logger);

  ip::host host(argv[1]);

  std::vector<ip::address> resolved = TRY(dns.resolve(host));

  if (resolved.empty()) {
    BNL_LOG_ERROR(&logger, "Failed to resolve {}", host);
    return errc::unknown;
  }

  BNL_LOG_INFO(
    &logger, "Host {} resolved to the following IP addresses: ", host);

  for (const ip::address &address : resolved) {
    BNL_LOG_INFO(&logger, "{}", address);
  }

  ip::address address = resolved[0];
  ip::port port = static_cast<uint16_t>(std::stoul(argv[2]));
  ip::endpoint peer(address, port);

  client client(host, peer, &logger);

  http3::request::handle request = TRY(client.request());

  TRY(request.header({ ":method", "GET" }));
  TRY(request.header({ ":scheme", "https" }));
  TRY(request.header({ ":authority", host.name() }));
  TRY(request.header({ ":path", "/index.html" }));

  TRY(request.start());
  TRY(request.fin());

  std::vector<http3::header> headers;
  base::buffer body;

  TRY(client.run([&](http3::event event) -> result<void> {
    switch (event) {
      case http3::event::type::settings:
        break;

      case http3::event::type::header:
        headers.emplace_back(std::move(event.header.header));
        break;

      case http3::event::type::body:
        body = base::buffer::concat(body, event.body.buffer);

        if (event.body.fin) {
          return base::error::finished;
        }

        break;
    }

    return success();
  }));

  for (const auto &header : headers) {
    std::cout << header << std::endl;
  }

  std::cout << std::endl;

  std::cout.write(reinterpret_cast<char *>(body.data()),
                  static_cast<std::streamsize>(body.size()));

  return success();
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
